# 开发计划

> 范围：2026-07-07 架构审计后的最小行动序列。  
> 目标：跑出第一个真实 AirSim -> WSL -> FlightCore 外部闭环悬停 episode，并形成可回放、可比较、可复现的数据地板。  
> 维护规则：本文件描述计划和验收，不记录会话流水或完成百分比；进度状态写入 PBOS `runtime/handoffs/UAVSingle.md`。

## 一句话目标

```text
在真实 AirSim 外部闭环中完成 30 s 悬停：
有 manifest.yaml，有 rosbag2，有时钟域记录，有判据脚本，有 PASS/FAIL 结果。
```

在这个目标完成前，冻结以下事项：

- 新增 `/uav/*` topic
- EscCmd/SystemHealth 改名
- `/aircraft/*` naming 迁移
- barometer、magnetometer、rangefinder、lidar 运行时落地
- Gazebo、Isaac、Pegasus adapter
- MAVLink Gateway
- RL、视觉、world model 接口实现
- C++ FlightBus 中间件

## 阶段总览

| 阶段 | 目标 | 完成判据 |
|---|---|---|
| P0 | 文档与权威边界收敛 | README、开发计划、文档索引、契约指针一致 |
| P1 | Mock 端到端 smoke | mock endpoint 收到 `/uav/actuator/esc_cmd` 对应 actuator packet；三时钟域偏差落盘 |
| P2 | AirSim motor API 探针 | `moveByMotorPWMsAsync` 可用；四电机顺序、旋转方向、`--motor-order` 定值 |
| P3 | 真实 AirSim 外部闭环 | AirSim -> WSL -> MATLAB FlightCore -> AirSim actuator 链路闭合 |
| P4 | 第一个悬停 episode | 30 s 悬停目录可整体拷走、回放、出图、判据 PASS/FAIL |
| P5 | V0 数据地板固化 | episode 格式、时钟容差、sequence、health、IsNew 策略写入契约/测试 |

## P1：Mock 端到端 smoke

目的不是验证飞行效果，而是证明当前开发期拓扑可跑通：

```text
Windows mock endpoint
  -> UDP 56000
  -> WSL aircraft_udp_bridge
  -> /aircraft/*
  -> WSL flightcore_runtime_adapter
  -> /uav/sensors/imu + /uav/sensors/gps
  -> Windows MATLAB FlightCore_ROS2_loop
  -> /uav/actuator/esc_cmd
  -> WSL actuator UDP sender
  -> UDP 56001
  -> Windows mock endpoint log
```

### 前置条件

- WSL workspace 使用原生路径 `~/uavsingle_ros2_ws`。
- WSL build 使用 `colcon build --merge-install --symlink-install`。
- Windows MATLAB 已注册 `flightcore_msgs`。
- `FlightCore_ROS2_loop.slx` 能解析六个 `/uav/*` topic。
- Windows endpoint 的 vendored schema 与 WSL 权威 schema 同步到 state、sensor_imu、sensor_gps、actuator 四类。
- WSL 侧安装 CycloneDDS（`ros-jazzy-cyclonedds`, `ros-jazzy-rmw-cyclonedds-cpp`）。MATLAB 私有 DDS 与 Fast-DDS 在 NAT 模式下不互通；CycloneDDS 通过 RTPS 线协议与 MATLAB DDS 双向匹配。
- WSL runtime 脚本设置 `RMW_IMPLEMENTATION=rmw_cyclonedds_cpp` 和 `PYTHONUNBUFFERED=1`。

### 必须记录

| 记录项 | 用途 |
|---|---|
| Windows endpoint wall-clock timestamp | simulator endpoint 时钟 |
| WSL ROS time / node receive time | ROS2 runtime 时钟 |
| Simulink sample time / ROS block receive time | FlightCore 开发脚手架时钟 |
| packet `sequence` | 丢包、乱序、新鲜度判断 |
| actuator packet timestamp + sequence | 闭环回传证据 |

### 完成判据

- mock endpoint 日志出现 actuator packet。
- `/uav/sensors/imu`、`/uav/sensors/gps`、`/uav/actuator/esc_cmd` 可被 WSL 侧 `ros2 topic echo` 或 rosbag2 捕获。
- 至少保存一次三时钟域偏差记录。
- 无 `/aircraft/*` topic 跨 Windows <-> WSL2 DDS。
- FlightCore 侧仍无 AirSim/ROS2/DDS API 符号泄漏。

## P2：AirSim motor API 探针

目的：不要把整条 actuator 主路径押在未验证 API 上。

### 探针内容

1. 连接 AirSim `Drone1`。
2. 检查 Python client 是否暴露 `moveByMotorPWMsAsync`。
3. 依次给四个 motor index 一个小阶跃，其余电机保持低值。
4. 记录飞行器姿态/转子响应。
5. 确定 FlightCore `MotorCmd[4]` 到 AirSim API 参数顺序的 `--motor-order`。
6. 记录不支持时的 fallback 行为，不把 fallback 当作闭环成功。

### 完成判据

- API 存在并可调用。
- 四电机响应能区分。
- `--motor-order` 写入 endpoint 配置或运行命令。
- 如果 API 不可用，计划切换到替代 simulator endpoint，不继续伪造执行器级闭环。

## P3：真实 AirSim 外部闭环

目的：把 mock 链路替换为真实 AirSim endpoint，同时保持最小变量。

### 约束

- 仍只使用 state、sensor_imu、sensor_gps、actuator 四类 packet。
- 不引入 barometer、magnetometer、rangefinder、lidar。
- 不引入 Gazebo/Isaac/Pegasus。
- 不改 `/uav/*` topic 名称。
- 不改 EscCmd/SystemHealth 命名。
- 不把 truth/state 作为 FlightCore 传感器输入；truth 只用于评估。

### 完成判据

- AirSim endpoint 能稳定发送 state、sensor_imu、sensor_gps。
- WSL runtime adapter 能发布 `/uav/sensors/imu` 和 `/uav/sensors/gps`。
- MATLAB FlightCore 能发布 `/uav/actuator/esc_cmd`。
- AirSim endpoint 收到 actuator packet 并调用 motor API。
- 断连、超时、旧样本重复时不保持无限期陈旧控制量。

## P4：第一个悬停 episode

### 目录约定

episode 产物不纳入 git，默认落在本地 `episodes/` 或 PBOS 指定实验目录。

```text
episodes/
└── YYYYMMDD_HHMMSS_airsim_hover_v0/
    ├── manifest.yaml
    ├── rosbag2/
    ├── endpoint.log
    ├── matlab.log
    ├── clock_offsets.csv
    ├── metrics.json
    └── plots/
```

### `manifest.yaml` 最小字段

```yaml
episode_id: YYYYMMDD_HHMMSS_airsim_hover_v0
project_git:
  windows_repo: <commit>
  wsl_repo: <commit>
runtime:
  simulator: AirSim
  vehicle: Drone1
  ros_distro: Jazzy
  matlab: R2025b
topology:
  endpoint_host: Windows
  ros2_runtime_host: WSL2
  flightcore_host: Windows MATLAB
dds_exception: /uav/* development-only
topics:
  recorded:
    - /aircraft/state
    - /aircraft/imu
    - /aircraft/gps
    - /uav/sensors/imu
    - /uav/sensors/gps
    - /uav/actuator/esc_cmd
    - /uav/estimator/state
    - /uav/health/status
criteria:
  duration_sec: 30
  result: pending
```

### PASS/FAIL 最小判据

| 类别 | 判据 |
|---|---|
| 可运行 | episode 至少持续 30 s；链路无未恢复崩溃 |
| 可记录 | rosbag2、manifest、endpoint log、clock_offsets 均存在 |
| 可回放 | `ros2 bag play` 能重放记录 topic |
| 可比较 | 生成 truth-vs-estimate 误差与 actuator 曲线 |
| 可复现 | manifest 中记录 Windows/WSL 两侧 git commit 与关键运行参数 |
| 飞行表现 | 高度、姿态、位置误差不持续发散，阈值在首次有效波形后冻结 |

## P5：V0 数据地板固化

在 P4 出现第一个有效 episode 后，再把实测结果反写为契约和测试：

- `sequence` 递增策略和 wrap 策略。
- 三时钟域映射与容差。
- `IsNew` freshness 阈值。
- `/uav/health/status` 最小字段。
- episode manifest 正式 schema。
- rosbag2 topic 最小记录集。
- truth-vs-estimate 指标脚本。

这一步之前不要提前扩大接口面；先拿到真实数据，再冻结阈值。

## 风险控制

| 风险 | 控制 |
|---|---|
| DDS 跨界不稳定 | 只允许 `/uav/*` 六 topic 跨界；先 mock smoke，再真实 AirSim |
| AirSim API 不支持 motor PWM | P2 独立探针，失败则换 endpoint 策略 |
| 三时钟域混乱 | P1 起即记录 clock_offsets，不等到真实飞行后补 |
| schema 权威漂移 | WSL 为权威，Windows 只 vendor 当前 endpoint 四类 schema |
| 文档再次膨胀 | README 只做入口，契约只写边界，计划只写执行，状态只进 PBOS |
| 接口冒进 | 冻结清单解冻前必须有 episode 证据或明确里程碑触发 |

## 解冻规则

任一冻结项要进入主线，必须同时满足：

1. 已完成 P4 第一个可回放 episode。
2. 该项能直接提升可运行、可复现、可记录、可比较、可扩展中的至少一项。
3. 有明确验收判据。
4. 对 FlightCore 边界的影响已在 `docs/contracts/` 中写清。
5. 变更前已补测试或 smoke 验证脚本。
