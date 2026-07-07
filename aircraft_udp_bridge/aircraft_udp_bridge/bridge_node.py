from __future__ import annotations

import json
import socket
import threading
import time
from typing import Any

import rclpy
from builtin_interfaces.msg import Time
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Imu
from std_msgs.msg import String

from aircraft_udp_bridge.protocol import (
    ProtocolError,
    decode_datagram,
    encode_packet,
    make_control_packet,
    validate_control_packet,
    validate_state_packet,
)


class AircraftUdpBridge(Node):
    def __init__(self) -> None:
        super().__init__("aircraft_udp_bridge")

        self.declare_parameter("state_bind_host", "0.0.0.0")
        self.declare_parameter("state_bind_port", 56000)
        self.declare_parameter("control_target_host", "")
        self.declare_parameter("control_target_port", 56001)
        self.declare_parameter("state_topic", "/aircraft/state")
        self.declare_parameter("imu_topic", "/aircraft/imu")
        self.declare_parameter("control_topic", "/aircraft/control")
        self.declare_parameter("cmd_vel_topic", "/aircraft/cmd_vel")
        self.declare_parameter("cmd_vel_mode", "rate")
        self.declare_parameter("publish_imu", True)
        self.declare_parameter("use_packet_timestamp", True)
        self.declare_parameter("log_period_sec", 1.0)

        self._state_bind_host = self.get_parameter("state_bind_host").value
        self._state_bind_port = int(self.get_parameter("state_bind_port").value)
        self._control_target_host = str(self.get_parameter("control_target_host").value)
        self._control_target_port = int(self.get_parameter("control_target_port").value)
        self._cmd_vel_mode = str(self.get_parameter("cmd_vel_mode").value)
        self._publish_imu_enabled = bool(self.get_parameter("publish_imu").value)
        self._use_packet_timestamp = bool(self.get_parameter("use_packet_timestamp").value)
        self._log_period_sec = float(self.get_parameter("log_period_sec").value)

        sensor_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=10,
            reliability=ReliabilityPolicy.BEST_EFFORT,
            durability=DurabilityPolicy.VOLATILE,
        )
        control_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=10,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.VOLATILE,
        )

        self._state_pub = self.create_publisher(
            Odometry, str(self.get_parameter("state_topic").value), sensor_qos
        )
        self._imu_pub = self.create_publisher(
            Imu, str(self.get_parameter("imu_topic").value), sensor_qos
        )
        self.create_subscription(
            String,
            str(self.get_parameter("control_topic").value),
            self._control_json_callback,
            control_qos,
        )
        self.create_subscription(
            Twist,
            str(self.get_parameter("cmd_vel_topic").value),
            self._cmd_vel_callback,
            control_qos,
        )

        self._state_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._state_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._state_sock.bind((self._state_bind_host, self._state_bind_port))
        self._state_sock.settimeout(0.2)

        self._control_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._stop_event = threading.Event()

        self._last_log_time = time.monotonic()
        self._rx_count = 0
        self._tx_count = 0
        self._decode_errors = 0
        self._last_control_sequence = 0

        self._rx_thread = threading.Thread(target=self._udp_rx_loop, daemon=True)
        self._rx_thread.start()

        self.get_logger().info(
            "listening for state UDP on "
            f"{self._state_bind_host}:{self._state_bind_port}; "
            f"control target={self._control_target_host or '<auto>'}:"
            f"{self._control_target_port}"
        )

    def destroy_node(self) -> bool:
        self._stop_event.set()
        try:
            self._state_sock.close()
        except OSError:
            pass
        try:
            self._control_sock.close()
        except OSError:
            pass
        return super().destroy_node()

    def _udp_rx_loop(self) -> None:
        while not self._stop_event.is_set():
            try:
                data, addr = self._state_sock.recvfrom(8192)
            except socket.timeout:
                self._log_rates_if_due()
                continue
            except OSError:
                break

            try:
                packet = validate_state_packet(decode_datagram(data))
            except ProtocolError as exc:
                self._decode_errors += 1
                self.get_logger().warning(f"dropped UDP state packet: {exc}")
                continue

            if not self._control_target_host:
                self._control_target_host = addr[0]
                self.get_logger().info(
                    f"learned control target host {self._control_target_host}"
                )

            self._publish_state(packet)
            self._rx_count += 1
            self._log_rates_if_due()

    def _publish_state(self, packet: dict[str, Any]) -> None:
        stamp = self._stamp(packet["timestamp"])

        odom = Odometry()
        odom.header.stamp = stamp
        odom.header.frame_id = packet["frame_id"]
        odom.child_frame_id = packet["child_frame_id"]
        odom.pose.pose.position.x = packet["position"][0]
        odom.pose.pose.position.y = packet["position"][1]
        odom.pose.pose.position.z = packet["position"][2]
        odom.pose.pose.orientation.x = packet["orientation"][0]
        odom.pose.pose.orientation.y = packet["orientation"][1]
        odom.pose.pose.orientation.z = packet["orientation"][2]
        odom.pose.pose.orientation.w = packet["orientation"][3]
        odom.twist.twist.linear.x = packet["linear_velocity"][0]
        odom.twist.twist.linear.y = packet["linear_velocity"][1]
        odom.twist.twist.linear.z = packet["linear_velocity"][2]
        odom.twist.twist.angular.x = packet["angular_velocity"][0]
        odom.twist.twist.angular.y = packet["angular_velocity"][1]
        odom.twist.twist.angular.z = packet["angular_velocity"][2]
        self._state_pub.publish(odom)

        if self._publish_imu_enabled:
            imu = Imu()
            imu.header.stamp = stamp
            imu.header.frame_id = packet["child_frame_id"]
            imu.orientation = odom.pose.pose.orientation
            imu.angular_velocity = odom.twist.twist.angular
            imu.linear_acceleration_covariance[0] = -1.0
            self._imu_pub.publish(imu)

    def _stamp(self, timestamp: float) -> Time:
        if not self._use_packet_timestamp:
            return self.get_clock().now().to_msg()
        sec = int(timestamp)
        nanosec = int((timestamp - sec) * 1_000_000_000)
        msg = Time()
        msg.sec = sec
        msg.nanosec = max(0, min(nanosec, 999_999_999))
        return msg

    def _control_json_callback(self, msg: String) -> None:
        try:
            raw = json.loads(msg.data)
            if not isinstance(raw, dict):
                raise ProtocolError("control message must be a JSON object")
            packet = validate_control_packet(raw)
        except (json.JSONDecodeError, ProtocolError) as exc:
            self.get_logger().warning(f"dropped ROS2 JSON control: {exc}")
            return
        self._send_control(packet)

    def _cmd_vel_callback(self, msg: Twist) -> None:
        self._last_control_sequence += 1
        try:
            packet = make_control_packet(
                sequence=self._last_control_sequence,
                mode=self._cmd_vel_mode,
                throttle=msg.linear.z,
                roll=msg.angular.x,
                pitch=msg.angular.y,
                yaw=msg.angular.z,
            )
        except ProtocolError as exc:
            self.get_logger().warning(f"dropped cmd_vel control: {exc}")
            return
        self._send_control(packet)

    def _send_control(self, packet: dict[str, Any]) -> None:
        if not self._control_target_host:
            self.get_logger().warning(
                "no control target host yet; wait for first state packet or set "
                "control_target_host"
            )
            return
        target = (self._control_target_host, self._control_target_port)
        try:
            self._control_sock.sendto(encode_packet(packet), target)
        except OSError as exc:
            self.get_logger().error(f"failed to send UDP control to {target}: {exc}")
            return
        self._tx_count += 1
        self._log_rates_if_due()

    def _log_rates_if_due(self) -> None:
        now = time.monotonic()
        elapsed = now - self._last_log_time
        if elapsed < self._log_period_sec:
            return
        rx_rate = self._rx_count / elapsed
        tx_rate = self._tx_count / elapsed
        self.get_logger().info(
            f"udp rx={self._rx_count} ({rx_rate:.1f} Hz), "
            f"tx={self._tx_count} ({tx_rate:.1f} Hz), "
            f"decode_errors={self._decode_errors}"
        )
        self._rx_count = 0
        self._tx_count = 0
        self._decode_errors = 0
        self._last_log_time = now


def main() -> None:
    rclpy.init()
    node = AircraftUdpBridge()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
