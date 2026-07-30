#!/usr/bin/env python3
"""等待严格锁步 ROS 图完全匹配，但不调用 PRIME 或推进仿真时间。

三种模式分别验证 generated FlightCore、Gazebo/Coordinator 或完整匹配图。
这里检查 publisher/subscriber/service 数量，而不是读取数据；因此可安全用于
paused 零时刻世界。SensorBatchPublished/ObservationReady 两端必须同时存在，
否则启动门即使打开也无法形成消费端传感器屏障。
"""

import argparse
import time

import rclpy
from rclpy.node import Node


def simulink_ready(node: Node) -> bool:
    """确认 generated node 具备时钟、控制输出和观测消费端点。"""
    return (
        node.count_subscribers("/clock") > 0
        and node.count_publishers("/flightcore/gazebo/actuator_command") > 0
        and node.count_subscribers(
            "/flightcore/gazebo/sensor_batch_published"
        ) > 0
        and node.count_publishers(
            "/flightcore/gazebo/observation_ready"
        ) > 0
    )


def gazebo_ready(node: Node) -> bool:
    """确认 plant、Coordinator 和全部显式 epoch ACK 已双向匹配。"""
    nodes = {
        f"{namespace.rstrip('/')}/{name}" if namespace != "/" else f"/{name}"
        for name, namespace in node.get_node_names_and_namespaces()
    }
    services = {name for name, _ in node.get_service_names_and_types()}

    return (
        "/flightcore_gazebo_system" in nodes
        and "/flightcore_simulation_coordinator" in nodes
        and "/flightcore/gazebo/prime_session" in services
        and "/flightcore/gazebo/blocking_status" in services
        and "/flightcore/gazebo/start_coordinator" in services
        and node.count_subscribers(
            "/flightcore/gazebo/actuator_command"
        ) >= 2
        and node.count_publishers(
            "/flightcore/gazebo/imu"
        ) > 0
        and node.count_publishers(
            "/flightcore/gazebo/gps"
        ) > 0
        and node.count_publishers(
            "/flightcore/gazebo/sensor_batch_published"
        ) > 0
        and node.count_subscribers(
            "/flightcore/gazebo/observation_ready"
        ) > 0
        and node.count_publishers(
            "/flightcore/gazebo/command_cached"
        ) > 0
        and node.count_subscribers(
            "/flightcore/gazebo/command_cached"
        ) > 0
        and node.count_publishers(
            "/flightcore/gazebo/plant_step_done"
        ) > 0
        and node.count_subscribers(
            "/flightcore/gazebo/plant_step_done"
        ) > 0
        and node.count_publishers(
            "/flightcore/gazebo/result_ready"
        ) > 0
        and node.count_subscribers(
            "/flightcore/gazebo/result_ready"
        ) > 0
        and node.count_publishers(
            "/flightcore/gazebo/commit_release"
        ) > 0
        and node.count_subscribers(
            "/flightcore/gazebo/commit_release"
        ) > 0
    )

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--timeout-sec", type=float, default=30.0)
    parser.add_argument(
        "--mode",
        choices=("simulink", "gazebo", "matched"),
        default="gazebo",
    )
    args = parser.parse_args()

    rclpy.init()
    node = Node("flightcore_gazebo_readiness_probe")
    try:
        deadline = time.monotonic() + args.timeout_sec
        while rclpy.ok() and time.monotonic() < deadline:
            simulink_is_ready = simulink_ready(node)
            gazebo_is_ready = gazebo_ready(node)
            mode_ready = (
                (args.mode == "simulink" and simulink_is_ready)
                or (args.mode == "gazebo" and gazebo_is_ready)
                or (
                    args.mode == "matched"
                    and simulink_is_ready
                    and gazebo_is_ready
                )
            )
            if mode_ready:
                print(f"FLIGHTCORE_{args.mode.upper()}_READY")
                return
            rclpy.spin_once(node, timeout_sec=0.25)
        raise RuntimeError("FlightCore-Gazebo ROS graph readiness timeout")
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
