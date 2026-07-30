"""
手动连接已运行的 AirSim 实例，展示三个核心 API 的原始输出。

用法：
    python inspect_airsim.py [--host 127.0.0.1] [--port 41451] [--vehicle SimpleFlight]

依赖：
    pip install airsim msgpack-rpc-python
"""

from __future__ import annotations

import argparse
import sys


def _v3(v) -> str:
    """Vector3r / GeoPoint → 可读字符串。"""
    return f"({v.x_val:.6f}, {v.y_val:.6f}, {v.z_val:.6f})"


def _quat(q) -> str:
    """Quaternionr → 可读字符串。"""
    return f"(w={q.w_val:.6f}, x={q.x_val:.6f}, y={q.y_val:.6f}, z={q.z_val:.6f})"


# ── 展示函数 ──────────────────────────────────────────────────────────────

def show_multirotor_state(client, vehicle: str) -> None:
    print("=" * 64)
    print("  getMultirotorState  —  kinematics_estimated")
    print("=" * 64)
    state = client.getMultirotorState(vehicle_name=vehicle)
    kin = state.kinematics_estimated
    print(f"  position             {_v3(kin.position)}")
    print(f"  orientation          {_quat(kin.orientation)}")
    print(f"  linear_velocity      {_v3(kin.linear_velocity)}")
    print(f"  angular_velocity     {_v3(kin.angular_velocity)}")
    print()


def show_imu(client, vehicle: str) -> None:
    print("=" * 64)
    print("  getImuData")
    print("=" * 64)
    imu = client.getImuData(vehicle_name=vehicle)
    print(f"  orientation          {_quat(imu.orientation)}")
    print(f"  angular_velocity     {_v3(imu.angular_velocity)}")
    print(f"  linear_acceleration  {_v3(imu.linear_acceleration)}")
    print(f"  time_stamp           {imu.time_stamp}")
    print()


def show_gps(client, vehicle: str) -> None:
    print("=" * 64)
    print("  getGpsData")
    print("=" * 64)
    gps = client.getGpsData(vehicle_name=vehicle)
    gnss = gps.gnss
    print(f"  time_stamp           {gps.time_stamp}")
    try:
        print(f"  is_valid             {gps.is_valid}")
    except AttributeError:
        print(f"  is_valid             <不存在>")
    print(f"  gnss.geo_point       lat={gnss.geo_point.latitude:.8f}  "
          f"lon={gnss.geo_point.longitude:.8f}  alt={gnss.geo_point.altitude:.4f}")
    print(f"  gnss.velocity        {_v3(gnss.velocity)}")
    print(f"  gnss.eph (h-err)     {gnss.eph:.4f}")
    print(f"  gnss.epv (v-err)     {gnss.epv:.4f}")
    print(f"  gnss.fix_type        {gnss.fix_type}")
    print(f"  gnss.time_utc        {gnss.time_utc}")
    print()


# ── 主入口 ────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(
        description="展示 AirSim 三个核心 API 的原始输出",
    )
    parser.add_argument("--host", default="127.0.0.1", help="AirSim RPC 地址")
    parser.add_argument("--port", type=int, default=41451, help="AirSim RPC 端口")
    parser.add_argument("--vehicle", default="Drone1", help="载具名称")
    args = parser.parse_args()

    try:
        import airsim
    except ImportError:
        print("错误：未安装 airsim Python 包。", file=sys.stderr)
        print("  pip install airsim", file=sys.stderr)
        sys.exit(1)

    print(f"连接 AirSim @ {args.host}:{args.port}  vehicle={args.vehicle} ...")
    client = airsim.MultirotorClient(
        ip=args.host, port=args.port, timeout_value=10.0,
    )
    try:
        client.confirmConnection()
    except Exception as exc:
        print(f"连接失败：{exc}", file=sys.stderr)
        sys.exit(1)

    print("已连接，读取传感器数据...")
    print()

    show_multirotor_state(client, args.vehicle)
    show_imu(client, args.vehicle)
    show_gps(client, args.vehicle)


if __name__ == "__main__":
    main()
