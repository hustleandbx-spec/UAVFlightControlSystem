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
from sensor_msgs.msg import Imu, NavSatFix, NavSatStatus
from std_msgs.msg import String

from aircraft_udp_bridge.protocol import (
    ProtocolError,
    decode_datagram,
    encode_packet,
    make_control_packet,
    validate_control_packet,
    validate_sensor_gps_packet,
    validate_sensor_imu_packet,
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
        self.declare_parameter("gps_topic", "/aircraft/gps")
        self.declare_parameter("control_topic", "/aircraft/control")
        self.declare_parameter("cmd_vel_topic", "/aircraft/cmd_vel")
        self.declare_parameter("cmd_vel_mode", "rate")
        self.declare_parameter("publish_imu", True)
        self.declare_parameter("publish_gps", True)
        self.declare_parameter("use_packet_timestamp", True)
        self.declare_parameter("log_period_sec", 1.0)

        self._state_bind_host = self.get_parameter("state_bind_host").value
        self._state_bind_port = int(self.get_parameter("state_bind_port").value)
        self._control_target_host = str(self.get_parameter("control_target_host").value)
        self._control_target_port = int(self.get_parameter("control_target_port").value)
        self._cmd_vel_mode = str(self.get_parameter("cmd_vel_mode").value)
        self._publish_imu_enabled = bool(self.get_parameter("publish_imu").value)
        self._publish_gps_enabled = bool(self.get_parameter("publish_gps").value)
        self._use_packet_timestamp = bool(self.get_parameter("use_packet_timestamp").value)
        self._log_period_sec = float(self.get_parameter("log_period_sec").value)

        sensor_qos = QoSProfile(history=HistoryPolicy.KEEP_LAST, depth=10,
            reliability=ReliabilityPolicy.BEST_EFFORT, durability=DurabilityPolicy.VOLATILE)
        control_qos = QoSProfile(history=HistoryPolicy.KEEP_LAST, depth=10,
            reliability=ReliabilityPolicy.RELIABLE, durability=DurabilityPolicy.VOLATILE)

        self._state_pub = self.create_publisher(Odometry,
            str(self.get_parameter("state_topic").value), sensor_qos)
        self._imu_pub = self.create_publisher(Imu,
            str(self.get_parameter("imu_topic").value), sensor_qos)
        if self._publish_gps_enabled:
            self._gps_pub = self.create_publisher(NavSatFix,
                str(self.get_parameter("gps_topic").value), sensor_qos)
        else:
            self._gps_pub = None
        self.create_subscription(String,
            str(self.get_parameter("control_topic").value),
            self._control_json_callback, control_qos)
        self.create_subscription(Twist,
            str(self.get_parameter("cmd_vel_topic").value),
            self._cmd_vel_callback, control_qos)

        self._state_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._state_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._state_sock.bind((self._state_bind_host, self._state_bind_port))
        self._state_sock.settimeout(0.2)
        self._control_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._stop_event = threading.Event()

        self._last_log_time = time.monotonic()
        self._rx_state = 0; self._rx_imu = 0; self._rx_gps = 0
        self._tx_count = 0; self._decode_errors = 0
        self._last_control_sequence = 0
        self._discarded_unknown_packet_types = 0

        self._rx_thread = threading.Thread(target=self._udp_rx_loop, daemon=True)
        self._rx_thread.start()
        self.get_logger().info(
            f"bridge listening on {self._state_bind_host}:{self._state_bind_port}; "
            f"control target={self._control_target_host or chr(60)+'auto'+chr(62)}")

    def destroy_node(self) -> bool:
        self._stop_event.set()
        for s in (self._state_sock, self._control_sock):
            try: s.close()
            except OSError: pass
        return super().destroy_node()

    def _udp_rx_loop(self) -> None:
        while not self._stop_event.is_set():
            try:
                data, addr = self._state_sock.recvfrom(8192)
            except socket.timeout:
                self._log_rates_if_due(); continue
            except OSError:
                break
            try:
                raw = decode_datagram(data)
            except ProtocolError as e:
                self._decode_errors += 1
                self.get_logger().warning(f"dropped datagram: {e}"); continue
            pt = raw.get("packet_type")
            if not self._control_target_host and pt == "state":
                self._control_target_host = addr[0]
                self.get_logger().info(f"learned target {self._control_target_host}")
            try:
                if pt == "state":
                    self._publish_state(validate_state_packet(raw)); self._rx_state += 1
                elif pt == "sensor_imu":
                    self._publish_imu(validate_sensor_imu_packet(raw)); self._rx_imu += 1
                elif pt == "sensor_gps":
                    self._publish_gps(validate_sensor_gps_packet(raw)); self._rx_gps += 1
                else:
                    self._discarded_unknown_packet_types += 1
            except ProtocolError as e:
                self._decode_errors += 1
                self.get_logger().warning(f"dropped {pt}: {e}")
            self._log_rates_if_due()

    def _publish_state(self, p: dict) -> None:
        s = self._stamp(p["timestamp"])
        o = Odometry()
        o.header.stamp = s; o.header.frame_id = p["frame_id"]; o.child_frame_id = p["child_frame_id"]
        o.pose.pose.position.x = p["position"][0]; o.pose.pose.position.y = p["position"][1]; o.pose.pose.position.z = p["position"][2]
        o.pose.pose.orientation.x = p["orientation"][0]; o.pose.pose.orientation.y = p["orientation"][1]
        o.pose.pose.orientation.z = p["orientation"][2]; o.pose.pose.orientation.w = p["orientation"][3]
        o.twist.twist.linear.x = p["linear_velocity"][0]; o.twist.twist.linear.y = p["linear_velocity"][1]; o.twist.twist.linear.z = p["linear_velocity"][2]
        o.twist.twist.angular.x = p["angular_velocity"][0]; o.twist.twist.angular.y = p["angular_velocity"][1]; o.twist.twist.angular.z = p["angular_velocity"][2]
        self._state_pub.publish(o)

    def _publish_imu(self, p: dict) -> None:
        s = self._stamp(p["timestamp"])
        imu = Imu()
        imu.header.stamp = s; imu.header.frame_id = p.get("frame_id", "aircraft_frd")
        o = p.get("orientation", [0,0,0,1])
        imu.orientation.x = o[0]; imu.orientation.y = o[1]; imu.orientation.z = o[2]; imu.orientation.w = o[3]
        g = p["angular_velocity"]; imu.angular_velocity.x = g[0]; imu.angular_velocity.y = g[1]; imu.angular_velocity.z = g[2]
        a = p["linear_acceleration"]; imu.linear_acceleration.x = a[0]; imu.linear_acceleration.y = a[1]; imu.linear_acceleration.z = a[2]
        self._imu_pub.publish(imu)

    def _publish_gps(self, p: dict) -> None:
        if self._gps_pub is None: return
        s = self._stamp(p["timestamp"]); geo = p["geo_point"]
        f = NavSatFix()
        f.header.stamp = s; f.header.frame_id = "gps"
        f.latitude = geo["latitude"]; f.longitude = geo["longitude"]; f.altitude = geo["altitude"]
        f.status.service = NavSatStatus.SERVICE_GPS
        f.status.status = NavSatStatus.STATUS_FIX if p.get("is_valid", True) else NavSatStatus.STATUS_NO_FIX
        self._gps_pub.publish(f)

    def _stamp(self, ts: float) -> Time:
        if not self._use_packet_timestamp:
            return self.get_clock().now().to_msg()
        sec = int(ts); ns = int((ts - sec) * 1e9)
        msg = Time(); msg.sec = sec; msg.nanosec = max(0, min(ns, 999999999))
        return msg

    def _control_json_callback(self, msg: String) -> None:
        try:
            raw = json.loads(msg.data)
            if not isinstance(raw, dict): raise ProtocolError("not an object")
            pkt = validate_control_packet(raw)
        except (json.JSONDecodeError, ProtocolError) as e:
            self.get_logger().warning(f"dropped JSON control: {e}"); return
        self._send_control(pkt)

    def _cmd_vel_callback(self, msg: Twist) -> None:
        self._last_control_sequence += 1
        try:
            pkt = make_control_packet(sequence=self._last_control_sequence,
                mode=self._cmd_vel_mode, throttle=msg.linear.z,
                roll=msg.angular.x, pitch=msg.angular.y, yaw=msg.angular.z)
        except ProtocolError as e:
            self.get_logger().warning(f"dropped cmd_vel: {e}"); return
        self._send_control(pkt)

    def _send_control(self, pkt: dict) -> None:
        if not self._control_target_host:
            self.get_logger().warning("no control target host yet"); return
        try:
            self._control_sock.sendto(encode_packet(pkt),
                (self._control_target_host, self._control_target_port))
        except OSError as e:
            self.get_logger().error(f"sendto failed: {e}"); return
        self._tx_count += 1; self._log_rates_if_due()

    def _log_rates_if_due(self) -> None:
        now = time.monotonic(); el = now - self._last_log_time
        if el < self._log_period_sec: return
        self.get_logger().info(
            f"rx_state={self._rx_state} rx_imu={self._rx_imu} rx_gps={self._rx_gps} "
            + f"tx={self._tx_count} err={self._decode_errors} unk={self._discarded_unknown_packet_types}")
        self._rx_state = 0; self._rx_imu = 0; self._rx_gps = 0
        self._tx_count = 0; self._decode_errors = 0
        self._discarded_unknown_packet_types = 0; self._last_log_time = now


def main() -> None:
    rclpy.init()
    node = AircraftUdpBridge()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node(); rclpy.shutdown()


if __name__ == "__main__":
    main()
