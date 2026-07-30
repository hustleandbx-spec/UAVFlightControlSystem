"""
Windows 侧 AirSim UDP endpoint。

架构角色（见 docs/contracts/runtime_isolation.md）：
    AirSim(UE4) <--RPC--> endpoint.py <--UDP--> WSL aircraft_udp_bridge
                    127.0.0.1:41451           BridgeHost:56000（上行 state/IMU/GPS）
                                              0.0.0.0:56001  （下行 actuator）

上行：读 AirSim RPC → JSON 编码 → UDP 发送
下行：UDP 接收 → JSON 解码 → AirSim RPC 控制

UDP 协议 schema 权威在 WSL aircraft_udp_bridge/protocol.py，
本文件 vendor 一份副本（windows/protocol.py），完整定义见 protocol.md。
"""

from __future__ import annotations

import json
import logging
import re
import socket
import subprocess
import sys
import threading
import time
from pathlib import Path
from typing import Any, Sequence

# ── vendor 协议层 ─────────────────────────────────────────────────────
# protocol.py 权威在 WSL，Windows 侧 vendor 一份以便 endpoint 自包含编解码。
# 通过 sys.path 动态挂载，避免污染全局 Python 环境。
_PROTOCOL_DIR = Path(__file__).resolve().parent
if str(_PROTOCOL_DIR) not in sys.path:
    sys.path.insert(0, str(_PROTOCOL_DIR))
from protocol import (  # type: ignore[import-untyped]  # noqa: E402
    ProtocolError,
    decode_datagram,
    encode_packet,
    make_sensor_gps_packet,
    make_sensor_imu_packet,
    make_state_packet,
    validate_command_packet,
)


# ── AirSim 类型转换 ───────────────────────────────────────────────────
# AirSim API 返回嵌套结构体对象（msgpackrpc 反序列化而来），字段名以 _val 结尾。
# 转为纯 float 列表方便 JSON 序列化，同时抹平 AirSim 版本间结构体差异。

def _vec3(vector: Any) -> list[float]:
    """AirSim Vector3r → [x, y, z]"""
    return [float(vector.x_val), float(vector.y_val), float(vector.z_val)]


def _quat(quaternion: Any) -> list[float]:
    """AirSim Quaternionr → [qx, qy, qz, qw]"""
    return [
        float(quaternion.x_val),
        float(quaternion.y_val),
        float(quaternion.z_val),
        float(quaternion.w_val),
    ]


# ── 线程安全的指令缓冲区 ──────────────────────────────────────────────
# 控制指令从 WSL 通过 UDP 异步到达。LatestControl 缓存最新包，主循环每次
# tick 调用 take_new() 消费一次，同一条指令绝不重复执行。
#
# 设计要点：
#   - update() 由接收线程调用，take_new() 由主线程调用，用锁保护共享状态。
#   - 去重用 sequence + timestamp 双条件判断，因为 FlightCore 重启后
#     sequence 可能归零，仅靠 sequence 比较会误判为新指令。

class LatestControl:
    """缓存最新到达的控制指令包，支持线程安全的写入与去重消费。"""

    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._packet: dict[str, Any] | None = None  # 当前最新包体
        self._sequence = -1      # 上次已消费的 sequence，-1 确保首包一定被取走
        self._timestamp = float("-inf")  # 上次已消费的 timestamp

    def update(self, packet: dict[str, Any]) -> None:
        """写入最新收到的包（由接收线程调用）。"""
        with self._lock:
            self._packet = packet

    def take_new(self) -> dict[str, Any] | None:
        """取回比上次更新的包，否则返回 None。主循环每 tick 调用一次。"""
        with self._lock:
            packet = self._packet
        if packet is None:
            return None
        sequence = int(packet["sequence"])
        timestamp = float(packet.get("timestamp", 0.0))
        # FlightCore 可能重复发送 sequence=0（例如重启后），所以同时比较
        # timestamp，避免只收一包后 actuator 输出冻结。
        if sequence == self._sequence and timestamp <= self._timestamp:
            return None
        self._sequence = sequence
        self._timestamp = timestamp
        return packet


# ── 指令接收（独立线程）───────────────────────────────────────────────
# 主循环按固定频率（如 250 Hz）发送传感器数据。如果在主循环阻塞收包会引入
# 抖动，所以把控制 UDP 接收放到独立线程，延迟与传感器发送节拍解耦。

def control_rx_loop(
    *,
    bind_host: str,
    bind_port: int,
    latest_control: LatestControl,
    stop_event: threading.Event,
    stats: dict[str, int],
) -> None:
    """独立线程：持续监听 actuator/control UDP 包。

    Args:
        bind_host: 监听地址，通常 "0.0.0.0"（接受来自任意网卡的数据包）。
        bind_port: 监听端口。
        latest_control: 线程安全的指令缓存，收到有效包后写入。
        stop_event: 主线程通过此事件通知接收线程退出。
        stats: 共享统计字典，记录接收计数和错误计数。
    """
    # 创建 UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # SO_REUSEADDR：允许快速重启时立即绑定同一端口，避免 TIME_WAIT 阻塞
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind((bind_host, bind_port))
    # 200ms 超时：不阻塞死等，每隔 200ms 检查 stop_event 是否被设置
    sock.settimeout(0.2)
    logging.info("listening for control UDP on %s:%d", bind_host, bind_port)
    try:
        while not stop_event.is_set():
            try:
                # 8192 字节缓冲区远大于最大 UDP 包（~1KB），足够容纳任何合法包
                data, addr = sock.recvfrom(8192)
            except socket.timeout:
                # 超时只是回来检查 stop_event，不是错误
                continue
            except OSError:
                # socket 被关闭时触发，正常退出
                break
            # 解码 + 校验：非法包丢弃并记录，避免脏数据写入缓存
            try:
                packet = validate_command_packet(decode_datagram(data))
            except ProtocolError as exc:
                stats["err"] += 1
                logging.warning("dropped command packet from %s: %s", addr, exc)
                continue
            # 校验通过，更新最新指令
            latest_control.update(packet)
            stats["rx"] += 1
    finally:
        sock.close()


# ── WSL IP 自动探测 ───────────────────────────────────────────────────
# WSL2 eth0 IP 由 Hyper-V 虚拟交换机 DHCP 分配，同机器重启通常不变，但换
# 机器或虚拟交换机重建后会变。启动时探测即可，无需写在静态配置里。

def _detect_wsl_bridge_host(distro: str) -> str:
    """返回 WSL2 发行版 eth0 接口的 IPv4 地址。

    原理：在 WSL 内执行 hostname -I，解析第一个 IPv4 地址。
    这个地址就是 WSL2 虚拟机在 Hyper-V 虚拟交换机上的 IP，
    Windows 侧可以直接用这个 IP 向 WSL 发送 UDP 包。
    """
    # 在指定 WSL 发行版内执行 shell 命令，获取 eth0 的 IPv4 地址
    result = subprocess.run(
        ["wsl", "-d", distro, "--", "bash", "-c",
         "hostname -I | awk '{print $1; exit}'"],
        capture_output=True, text=True, timeout=5,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"wsl command failed (distro={distro}): {result.stderr}"
        )
    # 从输出中提取第一个匹配的 IPv4 地址（容错：输出可能含空格或换行）
    match = re.search(r'(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})', result.stdout)
    if not match:
        raise RuntimeError(
            f"Could not detect WSL IPv4 from distro '{distro}': "
            f"{result.stdout.strip()}"
        )
    return match.group(1)


# ── AirSim 连接 ───────────────────────────────────────────────────────

def connect_airsim(
    host: str, port: int, timeout: float, vehicle_name: str
) -> Any:
    """连接已运行的 AirSim 实例，启用 API 控制并解锁，返回 MultirotorClient。

    调用顺序：
        1. 创建 MultirotorClient（RPC 连接）
        2. confirmConnection() — 握手确认 AirSim 在线
        3. enableApiControl(True) — 接管控制权（AirSim 默认手动模式）
        4. armDisarm(True) — 解锁电机（必须先 enableApiControl）
    """
    try:
        import airsim
    except ImportError as exc:
        raise RuntimeError(
            "AirSim Python package is not importable."
        ) from exc

    # AirSim MultirotorClient 内部使用 msgpackrpc 协议
    client = airsim.MultirotorClient(
        ip=host, port=port, timeout_value=timeout,
    )
    client.confirmConnection()   # 握手验证：确保 AirSim 正在运行
    client.enableApiControl(True, vehicle_name=vehicle_name)  # 切换为 API 控制模式
    client.armDisarm(True, vehicle_name=vehicle_name)         # 解锁电机
    return client


# ── 传感器读取 ────────────────────────────────────────────────────────
# 每个函数调一个 AirSim API，将结果通过 vendor 协议层组装为校验后的 UDP JSON 包。无回退逻辑——API 失败直接抛异常中止。

def read_airsim_state(
    client: Any, vehicle_name: str, sequence: int
) -> dict[str, Any]:
    """读 AirSim 真值运动学 → state 包

    state 包不进 FlightCore 控制闭环，仅用于真值对比、日志和离线评估。
    使用 kinematics_estimated（AirSim 内部估计值），非 ground_truth。
    """
    state = client.getMultirotorState(vehicle_name=vehicle_name)
    kin = state.kinematics_estimated
    return make_state_packet(
        sequence=sequence,
        position=_vec3(kin.position),
        orientation=_quat(kin.orientation),
        linear_velocity=_vec3(kin.linear_velocity),
        angular_velocity=_vec3(kin.angular_velocity),
    )


def read_airsim_imu(
    client: Any, vehicle_name: str, sequence: int
) -> dict[str, Any]:
    """读 AirSim IMU → sensor_imu 包

    经 flightcore_runtime_adapter 映射为 ROS2 /uav/sensors/imu。
    source_id 固定为 0（单 IMU 场景）。
    """
    imu_data = client.getImuData(vehicle_name=vehicle_name)
    return make_sensor_imu_packet(
        sequence=sequence,
        source_id=0,
        orientation=_quat(imu_data.orientation),
        angular_velocity=_vec3(imu_data.angular_velocity),
        linear_acceleration=_vec3(imu_data.linear_acceleration),
    )


def read_airsim_gps(
    client: Any, vehicle_name: str, sequence: int
) -> dict[str, Any]:
    """读 AirSim GPS → sensor_gps 包

    经 flightcore_runtime_adapter 映射为 ROS2 /uav/sensors/gps。
    数据路径：gps_data.gnss.geo_point（WGS84 经纬高）+ gps_data.gnss.velocity。
    """
    gps_data = client.getGpsData(vehicle_name=vehicle_name)
    # AirSim GPS 数据结构：
    #   GpsBase::Output::{time_stamp, gnss, is_valid}          ← is_valid 在此层
    #   GnssReport::{geo_point, eph, epv, velocity, fix_type, time_utc}
    return make_sensor_gps_packet(
        sequence=sequence,
        source_id=0,
        latitude=float(gps_data.gnss.geo_point.latitude),
        longitude=float(gps_data.gnss.geo_point.longitude),
        altitude=float(gps_data.gnss.geo_point.altitude),
        velocity=_vec3(gps_data.gnss.velocity),
        is_valid=bool(gps_data.is_valid),
    )


# ── 控制指令执行 ──────────────────────────────────────────────────────
# 将收到的控制包写回 AirSim。两种模式：
#   actuator  — 直接电机 PWM（闭环主路径）
#   control   — 角度/角速率 + 油门（legacy 调试路径）

def _ordered_motor_cmd(
    packet: dict[str, Any], motor_order: Sequence[int]
) -> list[float]:
    """将 FlightCore 电机顺序重排为 AirSim 的电机布局。

    FlightCore 使用固定电机顺序（如 FRD 坐标系），AirSim 有自己的编号约定。
    motor_order[i] 表示 AirSim 第 i 个电机对应 FlightCore motor_cmd 的第几个索引。
    例如 motor_order=[0,2,1,3] 表示 AirSim 电机 1→cmd[0], 电机 2→cmd[2], 以此类推。
    """
    motor_cmd = [float(item) for item in packet["motor_cmd"]]
    return [motor_cmd[index] for index in motor_order]


def apply_control(
    client: Any,
    packet: dict[str, Any],
    duration: float,
    vehicle_name: str,
    motor_order: Sequence[int],
) -> None:
    """将 UDP 控制包转换为 AirSim API 调用。

    Args:
        client: AirSim MultirotorClient 实例。
        packet: 已验证的控制包（actuator 或 control）。
        duration: 指令持续秒数，AirSim 在此时间后自动悬停。
        vehicle_name: AirSim 中飞行器的名字。
        motor_order: 电机顺序映射表。
    """
    packet_type = str(packet["packet_type"])

    # ── actuator 路径（闭环主路径） ──
    # 直接发送归一化电机 PWM，不经过角度/角速率中间层。
    # 这条路径是 FlightCore → ROS2 → UDP → AirSim 的唯一闭环链路。
    if packet_type == "actuator":
        motor_cmd = _ordered_motor_cmd(packet, motor_order)
        command = getattr(client, "moveByMotorPWMsAsync", None)
        if command is None:
            logging.warning(
                "received motor actuator packet, but this client has no "
                "moveByMotorPWMsAsync API; command ignored"
            )
            return
        logging.info(
            "airsim actuator mode=%s motor_cmd=[%.3f, %.3f, %.3f, %.3f]",
            packet["mode"],
            motor_cmd[0], motor_cmd[1], motor_cmd[2], motor_cmd[3],
        )
        # moveByMotorPWMsAsync：四个电机的 PWM 值 [0,1] + 持续时间（秒）
        command(
            motor_cmd[0], motor_cmd[1], motor_cmd[2], motor_cmd[3],
            duration,
            vehicle_name=vehicle_name,
        )
        return

    # ── control 路径（legacy 调试） ──
    # 用于手动调参和 mock 测试场景。通过 throttle/roll/pitch/yaw 控制，
    # 非 FlightCore 闭环主路径。
    # 将归一化节流阀钳位到 [0,1]，防止越界导致 AirSim 异常。
    throttle = max(0.0, min(1.0, float(packet["throttle"])))
    roll = float(packet["roll"])
    pitch = float(packet["pitch"])
    yaw = float(packet["yaw"])
    mode = str(packet["mode"])

    if mode == "rate":
        # rate 模式：直接控制角速率（roll/pitch/yaw 为 rad/s）
        # 优先使用 moveByAngleRatesThrottleAsync（AirSim 1.x+），
        # 回退到 moveByRollPitchYawrateThrottleAsync（旧版 API 命名）。
        command = getattr(client, "moveByAngleRatesThrottleAsync", None)
        if command is None:
            command = getattr(
                client, "moveByRollPitchYawrateThrottleAsync", None
            )
        if command is None:
            raise RuntimeError(
                "AirSim client has no supported rate-control API"
            )
        command(roll, pitch, yaw, throttle, duration,
                vehicle_name=vehicle_name)
    elif mode == "attitude":
        # attitude 模式：直接控制目标姿态角（roll/pitch/yaw 为弧度）
        command = getattr(client, "moveByRollPitchYawThrottleAsync", None)
        if command is None:
            raise RuntimeError(
                "AirSim client has no supported attitude-control API"
            )
        command(roll, pitch, yaw, throttle, duration,
                vehicle_name=vehicle_name)
    else:
        raise RuntimeError(f"unsupported control mode {mode}")


# ── 配置加载 ──────────────────────────────────────────────────────────

def _load_config(path: str) -> dict[str, Any]:
    """加载 JSON 配置文件，返回解析后的字典。"""
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


# ── 主循环 ────────────────────────────────────────────────────────────

def run() -> None:
    """endpoint 主入口：连接 AirSim → 启动接收线程 → 进入发送主循环。

    生命周期：
        1. 加载 episode_config.json 配置
        2. 初始化双路日志（stdout + endpoint.log）
        3. 连接 AirSim 并接管控制权
        4. 探测 WSL 桥接 IP
        5. 启动控制指令接收线程
        6. 进入主发送循环（state/IMU 每拍发送，GPS 降频发送）
        7. Ctrl+C 或异常时安全退出，释放 socket
    """

    # ── 配置加载 ──
    # episode_config.json 位于仓库根目录 orchestration/ 下。
    # 路径推导：当前文件在 bridge/windows/airsim_udp_endpoint.py，
    # parents[2] 即仓库根目录。
    _repo = Path(__file__).resolve().parents[2]
    cfg = _load_config(str(_repo / "orchestration" / "episode_config.json"))

    # ── 日志初始化 ──
    # 双路日志：
    #   stdout — 终端实时监控
    #   endpoint.log — 持久化到当前工作目录，供离线分析（配合 endpoint_meta.json）
    cwd = Path.cwd()
    handlers: list[logging.Handler] = [
        logging.StreamHandler(sys.stdout),
        logging.FileHandler(cwd / "endpoint.log", encoding="utf-8"),
    ]
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s %(message)s",
        handlers=handlers,
    )

    # ── 连接 AirSim ──
    # AirSim 运行在本地 UE4 进程中，通过 127.0.0.1 的 RPC 端口通信
    airsim_cfg = cfg["airsim"]
    vehicle_name = airsim_cfg["vehicle_name"]
    client = connect_airsim(
        host="127.0.0.1",
        port=airsim_cfg["rpc_port"],
        timeout=airsim_cfg["timeout_sec"],
        vehicle_name=vehicle_name,
    )

    # ── 解析运行时参数 ──
    ep = cfg["endpoint"]
    origin = cfg["origin"]
    # 自动探测 WSL 桥接 IP，避免硬编码
    bridge_host = _detect_wsl_bridge_host(cfg["wsl"]["distro"])
    logging.info("detected WSL bridge host: %s", bridge_host)
    state_target = (bridge_host, ep["state_port"])
    motor_order: Sequence[int] = ep["motor_order"]

    # ── 频率计算 ──
    # 使用累加 next_tick 而非每次 now+period，避免浮点累积导致频率漂移
    rate_hz = float(ep["rate_hz"])
    gps_rate_hz = float(ep["gps_rate_hz"])
    period = 1.0 / rate_hz
    # GPS 降频：例如 state 250 Hz、GPS 10 Hz → divider = max(1, 25) = 25
    # 即每 25 拍发一次 GPS
    gps_divider = max(1, round(rate_hz / gps_rate_hz))

    control_duration = float(ep["control_duration_sec"])  # 单次控制指令的持续秒数
    log_period = float(ep["log_period_sec"])             # 统计日志输出间隔

    # ── 写入元数据（供离线分析） ──
    # endpoint_meta.json 记录本次运行的静态参数，配合 endpoint.log 做离线回放
    meta = {
        "bridge_host": bridge_host,
        "state_port": ep["state_port"],
        "control_port": ep["control_port"],
        "vehicle_name": vehicle_name,
        "rate_hz": rate_hz,
        "gps_rate_hz": gps_rate_hz,
        "origin": origin,
        "motor_order": motor_order,
    }
    (cwd / "endpoint_meta.json").write_text(
        json.dumps(meta, indent=2), encoding="utf-8"
    )

    # ── 启动：UDP socket + 控制接收线程 ──
    # state_sock：发送传感器数据到 WSL（上行），不需要 bind，OS 自动分配源端口
    state_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    latest_control = LatestControl()
    stop_event = threading.Event()  # 用于通知接收线程退出
    # 每个统计窗口内的计数器（periodic 日志后归零）
    stats = {
        "tx_state": 0, "tx_imu": 0, "tx_gps": 0,
        "rx": 0, "err": 0,
    }
    # 控制指令接收线程：daemon=True 确保主线程退出时自动终止
    rx_thread = threading.Thread(
        target=control_rx_loop,
        kwargs={
            "bind_host": "0.0.0.0",          # 监听所有网卡
            "bind_port": ep["control_port"],  # 下行端口，WSL 向此端口发包
            "latest_control": latest_control,
            "stop_event": stop_event,
            "stats": stats,
        },
        daemon=True,
    )
    rx_thread.start()

    # ── 主发送循环 ──
    # 各数据包使用独立 sequence，按各自频率递增
    seq_state = 0
    seq_imu = 0
    seq_gps = 0
    # 使用 time.monotonic() 而非 time.time()——不受系统时钟跳变影响
    start_time = time.monotonic()
    next_tick = start_time       # 下一拍的绝对时间（累加器模式）
    last_log_time = start_time   # 上次输出统计日志的时间

    logging.info(
        "sending state UDP to %s:%d at %.1f Hz",
        *state_target, rate_hz
    )
    try:
        while True:
            # ── 节拍控制（自旋等待直到下一拍） ──
            # 累加 next_tick 而非 now+period：
            #   如果用 now+period，单次循环耗时波动会导致实际频率偏低。
            #   用累加器保证长期平均频率精确等于 rate_hz。
            now = time.monotonic()
            if now < next_tick:
                # 最多睡 2ms 后重新检查（不让 sleep 跨过下一拍时刻）
                time.sleep(min(next_tick - now, 0.002))
                continue
            next_tick += period
            seq_state += 1
            seq_imu += 1

            # ── 读 AirSim + 编码 ──
            # state 和 IMU 每拍发送
            state = client.getMultirotorState(vehicle_name=vehicle_name)
            state_pkt = make_state_packet(
                sequence=seq_state,
                position=_vec3(state.kinematics_estimated.position),
                orientation=_quat(state.kinematics_estimated.orientation),
                linear_velocity=_vec3(state.kinematics_estimated.linear_velocity),
                angular_velocity=_vec3(state.kinematics_estimated.angular_velocity),
            )
            imu_pkt = read_airsim_imu(client, vehicle_name, seq_imu)

            # sendto 到 WSL bridge 的上行端口（state + IMU 共用端口）
            state_sock.sendto(encode_packet(state_pkt), state_target)
            state_sock.sendto(encode_packet(imu_pkt), state_target)
            stats["tx_state"] += 1
            stats["tx_imu"] += 1

            # ── GPS 降频发送 ──
            # 例如 state 250 Hz → GPS 10 Hz：每 25 拍发一次，避免无效高频 GPS
            if seq_state % gps_divider == 0:
                seq_gps += 1
                gps_pkt = read_airsim_gps(client, vehicle_name, seq_gps)
                state_sock.sendto(encode_packet(gps_pkt), state_target)
                stats["tx_gps"] += 1

            # ── 消费最新的控制指令 ──
            # take_new() 只在有新指令时返回非 None，不做重复执行
            control = latest_control.take_new()
            if control is not None:
                try:
                    apply_control(
                        client, control, control_duration,
                        vehicle_name, motor_order,
                    )
                except Exception as exc:
                    # 控制异常不中断主循环——传感器数据继续发送
                    stats["err"] += 1
                    logging.error("failed to apply control: %s", exc)

            # ── 周期性统计输出（对齐 WSL bridge 格式） ──
            if now - last_log_time >= log_period:
                logging.info(
                    "tx_state=%d tx_imu=%d tx_gps=%d rx=%d err=%d",
                    stats["tx_state"], stats["tx_imu"], stats["tx_gps"],
                    stats["rx"], stats["err"],
                )
                stats.update(tx_state=0, tx_imu=0, tx_gps=0, rx=0, err=0)
                last_log_time = now
    except KeyboardInterrupt:
        logging.info("stopping")
    finally:
        # 安全退出：
        #   1. 设置 stop_event → 接收线程退出 recvfrom 循环
        #   2. 关闭上行 socket → 释放端口
        stop_event.set()
        state_sock.close()


if __name__ == "__main__":
    run()
