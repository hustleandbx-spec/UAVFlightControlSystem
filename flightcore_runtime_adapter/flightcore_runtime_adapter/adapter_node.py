from __future__ import annotations

import math
import socket
import time
from typing import Sequence

import rclpy
from flightcore_msgs.msg import EscCmd, FlightCmd, Gps, Imu as FcImu
from nav_msgs.msg import Odometry
from rclpy.node import Node
from rclpy.qos import DurabilityPolicy, HistoryPolicy, QoSProfile, ReliabilityPolicy
from sensor_msgs.msg import Imu as RosImu
from sensor_msgs.msg import NavSatFix

from aircraft_udp_bridge.protocol import ProtocolError, encode_packet
from flightcore_runtime_adapter.mapping import (
    LocalOrigin,
    Stamp,
    esc_cmd_to_actuator_packet,
    flight_cmd_to_contract,
    imu_to_contract,
    navsat_to_contract,
    odometry_to_debug_gps_contract,
    stamp_to_seconds,
)


class FlightCoreRuntimeAdapter(Node):
    def __init__(self) -> None:
        super().__init__("flightcore_runtime_adapter")

        self.declare_parameter("aircraft_state_topic", "/aircraft/state")
        self.declare_parameter("aircraft_imu_topic", "/aircraft/imu")
        self.declare_parameter("aircraft_gps_topic", "/aircraft/gps")
        self.declare_parameter("uav_imu_topic", "/uav/sensors/imu")
        self.declare_parameter("uav_gps_topic", "/uav/sensors/gps")
        self.declare_parameter("flight_cmd_topic", "/uav/cmd/flight")
        self.declare_parameter("esc_cmd_topic", "/uav/actuator/esc_cmd")

        self.declare_parameter("source_id", 1)
        self.declare_parameter("imu_timeout_sec", 0.2)
        self.declare_parameter("gps_timeout_sec", 1.0)
        self.declare_parameter("publish_invalid_on_timeout", True)
        self.declare_parameter("timeout_check_period_sec", 0.2)

        self.declare_parameter("gps_fallback_from_state", False)
        self.declare_parameter("local_origin_lat_deg", 47.641468)
        self.declare_parameter("local_origin_lon_deg", -122.140165)
        self.declare_parameter("local_origin_alt_m", 122.0)

        self.declare_parameter("publish_default_flight_cmd", True)
        self.declare_parameter("flight_cmd_rate_hz", 50.0)
        self.declare_parameter("flight_cmd_mode", 1)
        self.declare_parameter("flight_cmd_valid", True)
        self.declare_parameter("flight_cmd_position_ned_m", [0.0, 0.0, -2.0])
        self.declare_parameter("flight_cmd_velocity_ned_mps", [0.0, 0.0, 0.0])
        self.declare_parameter("flight_cmd_yaw_rad", 0.0)

        self.declare_parameter("actuator_target_host", "")
        self.declare_parameter("actuator_target_port", 56001)

        self._source_id = int(self.get_parameter("source_id").value)
        self._imu_timeout_sec = float(self.get_parameter("imu_timeout_sec").value)
        self._gps_timeout_sec = float(self.get_parameter("gps_timeout_sec").value)
        self._publish_invalid_on_timeout = bool(
            self.get_parameter("publish_invalid_on_timeout").value
        )
        self._gps_fallback_from_state = bool(
            self.get_parameter("gps_fallback_from_state").value
        )
        self._local_origin = LocalOrigin(
            lat_deg=float(self.get_parameter("local_origin_lat_deg").value),
            lon_deg=float(self.get_parameter("local_origin_lon_deg").value),
            alt_m=float(self.get_parameter("local_origin_alt_m").value),
        )
        self._publish_default_flight_cmd = bool(
            self.get_parameter("publish_default_flight_cmd").value
        )
        self._flight_cmd_rate_hz = float(self.get_parameter("flight_cmd_rate_hz").value)
        self._flight_cmd_mode = int(self.get_parameter("flight_cmd_mode").value)
        self._flight_cmd_valid = bool(self.get_parameter("flight_cmd_valid").value)
        self._flight_cmd_position = self._param_vec3("flight_cmd_position_ned_m")
        self._flight_cmd_velocity = self._param_vec3("flight_cmd_velocity_ned_mps")
        self._flight_cmd_yaw = float(self.get_parameter("flight_cmd_yaw_rad").value)

        self._actuator_target_host = str(self.get_parameter("actuator_target_host").value)
        self._actuator_target_port = int(self.get_parameter("actuator_target_port").value)
        self._actuator_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        sensor_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=10,
            reliability=ReliabilityPolicy.BEST_EFFORT,
            durability=DurabilityPolicy.VOLATILE,
        )
        command_qos = QoSProfile(
            history=HistoryPolicy.KEEP_LAST,
            depth=10,
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.VOLATILE,
        )

        self._imu_pub = self.create_publisher(
            FcImu, str(self.get_parameter("uav_imu_topic").value), sensor_qos
        )
        self._gps_pub = self.create_publisher(
            Gps, str(self.get_parameter("uav_gps_topic").value), sensor_qos
        )
        self._flight_cmd_pub = self.create_publisher(
            FlightCmd, str(self.get_parameter("flight_cmd_topic").value), command_qos
        )

        self.create_subscription(
            RosImu,
            str(self.get_parameter("aircraft_imu_topic").value),
            self._imu_callback,
            sensor_qos,
        )
        self.create_subscription(
            Odometry,
            str(self.get_parameter("aircraft_state_topic").value),
            self._state_callback,
            sensor_qos,
        )
        self.create_subscription(
            NavSatFix,
            str(self.get_parameter("aircraft_gps_topic").value),
            self._gps_callback,
            sensor_qos,
        )
        self.create_subscription(
            EscCmd,
            str(self.get_parameter("esc_cmd_topic").value),
            self._esc_cmd_callback,
            command_qos,
        )

        self._imu_sequence = 0
        self._gps_sequence = 0
        self._flight_cmd_sequence = 0
        self._last_imu_rx_mono: float | None = None
        self._last_gps_rx_mono: float | None = None
        self._latest_imu_accel = [0.0, 0.0, 0.0]
        self._latest_imu_gyro = [0.0, 0.0, 0.0]
        self._latest_velocity_ned = [0.0, 0.0, 0.0]
        self._have_real_gps = False
        self._warned_no_actuator_target = False
        self._warned_debug_gps_fallback = False

        timeout_period = float(self.get_parameter("timeout_check_period_sec").value)
        if timeout_period > 0.0:
            self.create_timer(timeout_period, self._timeout_callback)
        if self._publish_default_flight_cmd:
            if self._flight_cmd_rate_hz <= 0.0:
                raise ValueError("flight_cmd_rate_hz must be positive")
            self.create_timer(1.0 / self._flight_cmd_rate_hz, self._flight_cmd_timer)

        self.get_logger().info(
            "adapter online: /aircraft/* observation input, /uav/* contract output, "
            f"actuator target={self._actuator_target_host or '<unset>'}:"
            f"{self._actuator_target_port}"
        )
        if self._gps_fallback_from_state:
            self.get_logger().warning(
                "GPS fallback from local state is enabled for debug only; prefer a real GPS topic."
            )

    def destroy_node(self) -> bool:
        try:
            self._actuator_sock.close()
        except OSError:
            pass
        return super().destroy_node()

    def _param_vec3(self, name: str) -> list[float]:
        value = self.get_parameter(name).value
        if not isinstance(value, (list, tuple)) or len(value) != 3:
            raise ValueError(f"{name} must be a three-element array")
        return [float(item) for item in value]

    def _now_sec(self) -> float:
        sec, nanosec = self.get_clock().now().seconds_nanoseconds()
        return float(sec) + float(nanosec) * 1.0e-9

    @staticmethod
    def _stamp_from_ros(stamp: object) -> Stamp:
        return Stamp(sec=int(stamp.sec), nanosec=int(stamp.nanosec))

    @staticmethod
    def _assign_stamp(target: object, stamp: Stamp) -> None:
        target.sec = int(stamp.sec)
        target.nanosec = int(stamp.nanosec)

    @staticmethod
    def _ros_vec3(vector: object) -> list[float]:
        return [float(vector.x), float(vector.y), float(vector.z)]

    def _imu_callback(self, msg: RosImu) -> None:
        self._imu_sequence += 1
        accel = self._ros_vec3(msg.linear_acceleration)
        gyro = self._ros_vec3(msg.angular_velocity)
        try:
            contract = imu_to_contract(
                stamp=self._stamp_from_ros(msg.header.stamp),
                now_sec=self._now_sec(),
                sequence=self._imu_sequence,
                source_id=self._source_id,
                valid=True,
                linear_acceleration=accel,
                angular_velocity=gyro,
            )
        except ValueError as exc:
            self.get_logger().warning(f"dropped aircraft IMU sample: {exc}")
            return
        self._latest_imu_accel = accel
        self._latest_imu_gyro = gyro
        self._last_imu_rx_mono = time.monotonic()
        self._publish_imu_contract(contract)

    def _state_callback(self, msg: Odometry) -> None:
        position_ned = self._ros_vec3(msg.pose.pose.position)
        self._latest_velocity_ned = self._ros_vec3(msg.twist.twist.linear)
        if not self._gps_fallback_from_state or self._have_real_gps:
            return
        if not self._warned_debug_gps_fallback:
            self.get_logger().warning(
                "using local state to synthesize GPS for debug; this is not a sensor model."
            )
            self._warned_debug_gps_fallback = True
        self._gps_sequence += 1
        try:
            contract = odometry_to_debug_gps_contract(
                stamp=self._stamp_from_ros(msg.header.stamp),
                now_sec=self._now_sec(),
                sequence=self._gps_sequence,
                source_id=self._source_id,
                valid=True,
                position_ned_m=position_ned,
                velocity_ned_mps=self._latest_velocity_ned,
                local_origin=self._local_origin,
            )
        except ValueError as exc:
            self.get_logger().warning(f"dropped debug GPS fallback sample: {exc}")
            return
        self._last_gps_rx_mono = time.monotonic()
        self._publish_gps_contract(contract)

    def _gps_callback(self, msg: NavSatFix) -> None:
        self._gps_sequence += 1
        self._have_real_gps = True
        valid = bool(msg.status.status >= 0)
        try:
            contract = navsat_to_contract(
                stamp=self._stamp_from_ros(msg.header.stamp),
                now_sec=self._now_sec(),
                sequence=self._gps_sequence,
                source_id=self._source_id,
                valid=valid,
                lat_deg=msg.latitude,
                lon_deg=msg.longitude,
                alt_m=msg.altitude,
                velocity_ned_mps=self._latest_velocity_ned,
            )
        except ValueError as exc:
            self.get_logger().warning(f"dropped aircraft GPS sample: {exc}")
            return
        self._last_gps_rx_mono = time.monotonic()
        self._publish_gps_contract(contract)

    def _flight_cmd_timer(self) -> None:
        self._flight_cmd_sequence += 1
        try:
            contract = flight_cmd_to_contract(
                stamp=Stamp(),
                now_sec=self._now_sec(),
                sequence=self._flight_cmd_sequence,
                source_id=self._source_id,
                valid=self._flight_cmd_valid,
                mode=self._flight_cmd_mode,
                position_ned_sp_m=self._flight_cmd_position,
                velocity_ned_sp_mps=self._flight_cmd_velocity,
                yaw_sp_rad=self._flight_cmd_yaw,
            )
        except ValueError as exc:
            self.get_logger().warning(f"dropped default flight command: {exc}")
            return
        msg = FlightCmd()
        self._assign_common_fields(msg, contract)
        msg.mode = int(contract["mode"])
        msg.position_ned_sp_m = list(contract["position_ned_sp_m"])
        msg.velocity_ned_sp_mps = list(contract["velocity_ned_sp_mps"])
        msg.yaw_sp_rad = float(contract["yaw_sp_rad"])
        self._flight_cmd_pub.publish(msg)

    def _esc_cmd_callback(self, msg: EscCmd) -> None:
        if not bool(msg.valid):
            self.get_logger().warning("dropped invalid FlightCore ESC command")
            return
        timestamp_sec = float(msg.timestamp_sec)
        if not math.isfinite(timestamp_sec) or timestamp_sec <= 0.0:
            timestamp_sec = stamp_to_seconds(self._stamp_from_ros(msg.stamp), self._now_sec())
        try:
            packet = esc_cmd_to_actuator_packet(
                timestamp_sec=timestamp_sec,
                sequence=int(msg.sequence),
                motor_cmd=list(msg.motor_cmd),
            )
        except (ValueError, ProtocolError) as exc:
            self.get_logger().warning(f"dropped FlightCore ESC command: {exc}")
            return
        self._send_actuator_packet(packet)

    def _send_actuator_packet(self, packet: dict[str, object]) -> None:
        if not self._actuator_target_host:
            if not self._warned_no_actuator_target:
                self.get_logger().warning(
                    "actuator_target_host is unset; motor UDP packets are not sent"
                )
                self._warned_no_actuator_target = True
            return
        target = (self._actuator_target_host, self._actuator_target_port)
        try:
            self._actuator_sock.sendto(encode_packet(packet), target)
        except OSError as exc:
            self.get_logger().error(f"failed to send actuator UDP to {target}: {exc}")

    def _timeout_callback(self) -> None:
        if not self._publish_invalid_on_timeout:
            return
        now_mono = time.monotonic()
        now_sec = self._now_sec()
        if self._last_imu_rx_mono is None or now_mono - self._last_imu_rx_mono > self._imu_timeout_sec:
            self._imu_sequence += 1
            contract = imu_to_contract(
                stamp=Stamp(),
                now_sec=now_sec,
                sequence=self._imu_sequence,
                source_id=self._source_id,
                valid=False,
                linear_acceleration=self._latest_imu_accel,
                angular_velocity=self._latest_imu_gyro,
            )
            self._publish_imu_contract(contract)

        if self._last_gps_rx_mono is None or now_mono - self._last_gps_rx_mono > self._gps_timeout_sec:
            self._gps_sequence += 1
            contract = navsat_to_contract(
                stamp=Stamp(),
                now_sec=now_sec,
                sequence=self._gps_sequence,
                source_id=self._source_id,
                valid=False,
                lat_deg=self._local_origin.lat_deg,
                lon_deg=self._local_origin.lon_deg,
                alt_m=self._local_origin.alt_m,
                velocity_ned_mps=self._latest_velocity_ned,
            )
            self._publish_gps_contract(contract)

    def _assign_common_fields(self, msg: object, contract: dict[str, object]) -> None:
        self._assign_stamp(msg.stamp, contract["stamp"])
        msg.timestamp_sec = float(contract["timestamp_sec"])
        msg.sequence = int(contract["sequence"])
        msg.source_id = int(contract["source_id"])
        msg.valid = bool(contract["valid"])

    def _publish_imu_contract(self, contract: dict[str, object]) -> None:
        msg = FcImu()
        self._assign_common_fields(msg, contract)
        msg.accel_mps2 = list(contract["accel_mps2"])
        msg.gyro_radps = list(contract["gyro_radps"])
        self._imu_pub.publish(msg)

    def _publish_gps_contract(self, contract: dict[str, object]) -> None:
        msg = Gps()
        self._assign_common_fields(msg, contract)
        msg.lat_deg = float(contract["lat_deg"])
        msg.lon_deg = float(contract["lon_deg"])
        msg.alt_m = float(contract["alt_m"])
        msg.velocity_ned_mps = list(contract["velocity_ned_mps"])
        self._gps_pub.publish(msg)


def main() -> None:
    rclpy.init()
    node = FlightCoreRuntimeAdapter()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()
