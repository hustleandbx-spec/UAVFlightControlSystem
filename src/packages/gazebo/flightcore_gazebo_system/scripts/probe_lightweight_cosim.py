#!/usr/bin/env python3
"""对 Coordinator 全局 epoch 屏障执行连续 iteration / sim time 探针。"""

import argparse
import statistics
import time

import rclpy
from rclpy.node import Node
from rclpy.qos import (
    DurabilityPolicy,
    HistoryPolicy,
    QoSProfile,
    ReliabilityPolicy,
)

from flightcore_gazebo_msgs.msg import ActuatorCommand
from flightcore_gazebo_msgs.srv import PrimeSession
from flightcore_msgs.msg import Imu


class LightweightProbe(Node):
    def __init__(self) -> None:
        super().__init__("flightcore_gazebo_lightweight_probe")
        self.prime_client = self.create_client(
            PrimeSession, "/flightcore/gazebo/prime_session"
        )
        self.command_pub = self.create_publisher(
            ActuatorCommand, "/flightcore/gazebo/actuator_command", 8
        )
        self.imu_samples = {}
        sensor_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=10,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.VOLATILE,
        )
        self.imu_sub = self.create_subscription(
            Imu,
            "/flightcore/gazebo/imu",
            self._on_imu,
            sensor_qos,
        )

    def _on_imu(self, message: Imu) -> None:
        self.imu_samples[message.sequence] = message

    def call_prime(self, request, timeout_sec: float):
        future = self.prime_client.call_async(request)
        rclpy.spin_until_future_complete(self, future, timeout_sec=timeout_sec)
        if not future.done() or future.result() is None:
            raise RuntimeError("服务调用超时或未返回结果")
        return future.result()

    def wait_for_imu(self, sequence: int, timeout_sec: float) -> Imu:
        deadline = time.monotonic() + timeout_sec
        while time.monotonic() < deadline:
            rclpy.spin_once(self, timeout_sec=0.05)
            if sequence in self.imu_samples:
                return self.imu_samples.pop(sequence)
        raise RuntimeError(f"等待 CommitRelease 后的 IMU[{sequence}] 超时")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--steps", type=int, default=1000)
    parser.add_argument("--session", type=int, default=2026072002)
    parser.add_argument("--motor", type=float, default=0.0)
    parser.add_argument("--timeout", type=float, default=10.0)
    args = parser.parse_args()

    rclpy.init()
    node = LightweightProbe()
    try:
        if not node.prime_client.wait_for_service(timeout_sec=args.timeout):
            raise RuntimeError("PrimeSession 服务不可用")
        endpoint_deadline = time.monotonic() + args.timeout
        while (
            node.command_pub.get_subscription_count() < 2
            and time.monotonic() < endpoint_deadline
        ):
            rclpy.spin_once(node, timeout_sec=0.1)
        if node.command_pub.get_subscription_count() < 2:
            raise RuntimeError("插件和 Coordinator 命令订阅端点未就绪")

        prime_request = PrimeSession.Request()
        prime_request.session_id = args.session
        prime_response = node.call_prime(prime_request, args.timeout)
        if not prime_response.success:
            raise RuntimeError(prime_response.status)
        prime_imu = node.wait_for_imu(0, args.timeout)
        if not prime_imu.valid:
            raise RuntimeError("PRIME IMU 无效")

        expected_iteration = prime_response.iteration
        expected_time_ns = prime_response.sim_time_ns
        latencies_ms = []

        for step_id in range(1, args.steps + 1):
            command = ActuatorCommand()
            command.source_step_id = step_id - 1
            command.target_step_id = step_id
            command.command_id = step_id
            command.valid_from_iteration = expected_iteration + 1
            command.armed = args.motor > 0.0
            command.valid = True
            command.actuator_values = [args.motor] * 4

            wall_start = time.perf_counter()
            node.command_pub.publish(command)
            response = node.wait_for_imu(step_id, args.timeout)
            latencies_ms.append((time.perf_counter() - wall_start) * 1000.0)

            expected_iteration += 1
            expected_time_ns += 1_000_000
            identity_ok = (
                response.valid
                and response.sequence == expected_iteration
                and round(response.timestamp_sec * 1_000_000_000)
                == expected_time_ns
            )
            if not identity_ok:
                raise RuntimeError(
                    f"step {step_id} 身份或时间不连续: {response}"
                )

        ordered = sorted(latencies_ms)

        def percentile(percent: float) -> float:
            index = min(len(ordered) - 1, round((len(ordered) - 1) * percent))
            return ordered[index]

        print(
            "LIGHTWEIGHT_COSIM_PROBE_PASS "
            f"steps={args.steps} iteration={expected_iteration} "
            f"sim_time_ns={expected_time_ns} "
            f"mean_ms={statistics.fmean(latencies_ms):.3f} "
            f"p50_ms={percentile(0.50):.3f} "
            f"p95_ms={percentile(0.95):.3f} "
            f"p99_ms={percentile(0.99):.3f} "
            f"max_ms={max(latencies_ms):.3f}"
        )
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
