"""Shared JSON UDP protocol helpers for aircraft state/control packets."""

from __future__ import annotations

import json
import math
import time
from typing import Any

PROTOCOL_VERSION = 1
STATE_PACKET = "state"
CONTROL_PACKET = "control"
ACTUATOR_PACKET = "actuator"
SENSOR_IMU_PACKET = "sensor_imu"
SENSOR_GPS_PACKET = "sensor_gps"
SENSOR_BAROMETER_PACKET = "sensor_barometer"
SENSOR_MAGNETOMETER_PACKET = "sensor_magnetometer"
SENSOR_RANGEFINDER_PACKET = "sensor_rangefinder"
SENSOR_LIDAR_PACKET = "sensor_lidar"
ENVIRONMENT_PACKET = "environment"
COLLISION_PACKET = "collision"
MOTOR_STATE_PACKET = "motor_state"
CONTROL_MODES = {"rate", "attitude"}
ACTUATOR_MODES = {"motor"}
OBSERVATION_PACKETS = {
    STATE_PACKET,
    SENSOR_IMU_PACKET,
    SENSOR_GPS_PACKET,
    SENSOR_BAROMETER_PACKET,
    SENSOR_MAGNETOMETER_PACKET,
    SENSOR_RANGEFINDER_PACKET,
    SENSOR_LIDAR_PACKET,
    ENVIRONMENT_PACKET,
    COLLISION_PACKET,
    MOTOR_STATE_PACKET,
}
MAX_DATAGRAM_BYTES = 8192


class ProtocolError(ValueError):
    """Raised when a UDP datagram does not match the fixed bridge schema."""


def _finite_float(name: str, value: Any) -> float:
    try:
        out = float(value)
    except (TypeError, ValueError) as exc:
        raise ProtocolError(f"{name} must be a number") from exc
    if not math.isfinite(out):
        raise ProtocolError(f"{name} must be finite")
    return out


def _uint(name: str, value: Any) -> int:
    if isinstance(value, bool):
        raise ProtocolError(f"{name} must be an integer")
    try:
        out = int(value)
    except (TypeError, ValueError) as exc:
        raise ProtocolError(f"{name} must be an integer") from exc
    if out < 0:
        raise ProtocolError(f"{name} must be non-negative")
    return out


def _vector(name: str, value: Any, length: int) -> list[float]:
    if not isinstance(value, (list, tuple)) or len(value) != length:
        raise ProtocolError(f"{name} must be a {length}-element array")
    return [_finite_float(f"{name}[{idx}]", item) for idx, item in enumerate(value)]


def _float_array(name: str, value: Any) -> list[float]:
    if not isinstance(value, (list, tuple)):
        raise ProtocolError(f"{name} must be an array")
    return [_finite_float(f"{name}[{idx}]", item) for idx, item in enumerate(value)]


def _int_array(name: str, value: Any) -> list[int]:
    if not isinstance(value, (list, tuple)):
        raise ProtocolError(f"{name} must be an array")
    return [_uint(f"{name}[{idx}]", item) for idx, item in enumerate(value)]


def _normalized_vector(name: str, value: Any, length: int) -> list[float]:
    out = _vector(name, value, length)
    for idx, item in enumerate(out):
        if item < 0.0 or item > 1.0:
            raise ProtocolError(f"{name}[{idx}] must be in [0.0, 1.0]")
    return out


def _bool(name: str, value: Any) -> bool:
    if not isinstance(value, bool):
        raise ProtocolError(f"{name} must be a boolean")
    return value


def _object(name: str, value: Any) -> dict[str, Any]:
    if not isinstance(value, dict):
        raise ProtocolError(f"{name} must be a JSON object")
    return value


def _pose(name: str, value: Any) -> dict[str, Any]:
    raw = _object(name, value)
    return {
        "position": _vector(f"{name}.position", raw.get("position"), 3),
        "orientation": _vector(f"{name}.orientation", raw.get("orientation"), 4),
    }


def _base_packet(packet: dict[str, Any], packet_type: str) -> dict[str, Any]:
    if packet.get("protocol_version") != PROTOCOL_VERSION:
        raise ProtocolError("protocol_version must be 1")
    if packet.get("packet_type") != packet_type:
        raise ProtocolError(f"packet_type must be {packet_type}")
    out = {
        "protocol_version": PROTOCOL_VERSION,
        "packet_type": packet_type,
        "timestamp": _finite_float("timestamp", packet.get("timestamp")),
        "sequence": _uint("sequence", packet.get("sequence")),
    }
    if "source_id" in packet:
        out["source_id"] = _uint("source_id", packet.get("source_id"))
    return out


def encode_packet(packet: dict[str, Any]) -> bytes:
    return json.dumps(packet, separators=(",", ":"), ensure_ascii=True).encode("utf-8")


def decode_datagram(data: bytes) -> dict[str, Any]:
    if len(data) > MAX_DATAGRAM_BYTES:
        raise ProtocolError(f"datagram exceeds {MAX_DATAGRAM_BYTES} bytes")
    try:
        payload = json.loads(data.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise ProtocolError("datagram is not valid UTF-8 JSON") from exc
    if not isinstance(payload, dict):
        raise ProtocolError("packet must be a JSON object")
    return payload


def make_state_packet(
    *,
    timestamp: float | None = None,
    sequence: int,
    position: list[float] | tuple[float, float, float],
    orientation: list[float] | tuple[float, float, float, float],
    linear_velocity: list[float] | tuple[float, float, float],
    angular_velocity: list[float] | tuple[float, float, float],
    frame_id: str = "map_ned",
    child_frame_id: str = "aircraft_frd",
) -> dict[str, Any]:
    packet = {
        "protocol_version": PROTOCOL_VERSION,
        "packet_type": STATE_PACKET,
        "timestamp": time.time() if timestamp is None else timestamp,
        "sequence": sequence,
        "frame_id": frame_id,
        "child_frame_id": child_frame_id,
        "position": list(position),
        "orientation": list(orientation),
        "linear_velocity": list(linear_velocity),
        "angular_velocity": list(angular_velocity),
    }
    return validate_state_packet(packet)


def make_control_packet(
    *,
    timestamp: float | None = None,
    sequence: int,
    mode: str,
    throttle: float,
    roll: float,
    pitch: float,
    yaw: float,
) -> dict[str, Any]:
    packet = {
        "protocol_version": PROTOCOL_VERSION,
        "packet_type": CONTROL_PACKET,
        "timestamp": time.time() if timestamp is None else timestamp,
        "sequence": sequence,
        "mode": mode,
        "throttle": throttle,
        "roll": roll,
        "pitch": pitch,
        "yaw": yaw,
    }
    return validate_control_packet(packet)


def make_actuator_packet(
    *,
    timestamp: float | None = None,
    sequence: int,
    motor_cmd: list[float] | tuple[float, float, float, float],
    mode: str = "motor",
) -> dict[str, Any]:
    packet = {
        "protocol_version": PROTOCOL_VERSION,
        "packet_type": ACTUATOR_PACKET,
        "timestamp": time.time() if timestamp is None else timestamp,
        "sequence": sequence,
        "mode": mode,
        "motor_cmd": list(motor_cmd),
    }
    return validate_actuator_packet(packet)


def validate_state_packet(packet: dict[str, Any]) -> dict[str, Any]:
    out = {
        **_base_packet(packet, STATE_PACKET),
        "frame_id": str(packet.get("frame_id", "map_ned")),
        "child_frame_id": str(packet.get("child_frame_id", "aircraft_frd")),
        "position": _vector("position", packet.get("position"), 3),
        "orientation": _vector("orientation", packet.get("orientation"), 4),
        "linear_velocity": _vector("linear_velocity", packet.get("linear_velocity"), 3),
        "angular_velocity": _vector("angular_velocity", packet.get("angular_velocity"), 3),
    }
    return out


def validate_observation_packet(packet: dict[str, Any]) -> dict[str, Any]:
    packet_type = packet.get("packet_type")
    if packet_type == STATE_PACKET:
        return validate_state_packet(packet)
    if packet_type == SENSOR_IMU_PACKET:
        return validate_sensor_imu_packet(packet)
    if packet_type == SENSOR_GPS_PACKET:
        return validate_sensor_gps_packet(packet)
    if packet_type == SENSOR_BAROMETER_PACKET:
        return validate_sensor_barometer_packet(packet)
    if packet_type == SENSOR_MAGNETOMETER_PACKET:
        return validate_sensor_magnetometer_packet(packet)
    if packet_type == SENSOR_RANGEFINDER_PACKET:
        return validate_sensor_rangefinder_packet(packet)
    if packet_type == SENSOR_LIDAR_PACKET:
        return validate_sensor_lidar_packet(packet)
    if packet_type == ENVIRONMENT_PACKET:
        return validate_environment_packet(packet)
    if packet_type == COLLISION_PACKET:
        return validate_collision_packet(packet)
    if packet_type == MOTOR_STATE_PACKET:
        return validate_motor_state_packet(packet)
    raise ProtocolError("packet_type must be a known observation packet type")


def validate_command_packet(packet: dict[str, Any]) -> dict[str, Any]:
    packet_type = packet.get("packet_type")
    if packet_type == CONTROL_PACKET:
        return validate_control_packet(packet)
    if packet_type == ACTUATOR_PACKET:
        return validate_actuator_packet(packet)
    raise ProtocolError("packet_type must be control or actuator")


def validate_control_packet(packet: dict[str, Any]) -> dict[str, Any]:
    base = _base_packet(packet, CONTROL_PACKET)

    mode = str(packet.get("mode", ""))
    if mode not in CONTROL_MODES:
        raise ProtocolError("mode must be rate or attitude")

    throttle = _finite_float("throttle", packet.get("throttle"))
    if throttle < 0.0 or throttle > 1.0:
        raise ProtocolError("throttle must be in [0.0, 1.0]")

    out = {
        **base,
        "mode": mode,
        "throttle": throttle,
        "roll": _finite_float("roll", packet.get("roll")),
        "pitch": _finite_float("pitch", packet.get("pitch")),
        "yaw": _finite_float("yaw", packet.get("yaw")),
    }
    return out


def validate_actuator_packet(packet: dict[str, Any]) -> dict[str, Any]:
    base = _base_packet(packet, ACTUATOR_PACKET)

    mode = str(packet.get("mode", ""))
    if mode not in ACTUATOR_MODES:
        raise ProtocolError("mode must be motor")

    out = {
        **base,
        "mode": mode,
        "motor_cmd": _normalized_vector("motor_cmd", packet.get("motor_cmd"), 4),
    }
    return out


def validate_sensor_imu_packet(packet: dict[str, Any]) -> dict[str, Any]:
    out = {
        **_base_packet(packet, SENSOR_IMU_PACKET),
        "frame_id": str(packet.get("frame_id", "aircraft_frd")),
        "orientation": _vector("orientation", packet.get("orientation"), 4),
        "angular_velocity": _vector("angular_velocity", packet.get("angular_velocity"), 3),
        "linear_acceleration": _vector("linear_acceleration", packet.get("linear_acceleration"), 3),
    }
    return out


def validate_sensor_gps_packet(packet: dict[str, Any]) -> dict[str, Any]:
    geo = _object("geo_point", packet.get("geo_point"))
    out = {
        **_base_packet(packet, SENSOR_GPS_PACKET),
        "geo_point": {
            "latitude": _finite_float("geo_point.latitude", geo.get("latitude")),
            "longitude": _finite_float("geo_point.longitude", geo.get("longitude")),
            "altitude": _finite_float("geo_point.altitude", geo.get("altitude")),
        },
        "velocity": _vector("velocity", packet.get("velocity"), 3),
        "is_valid": _bool("is_valid", packet.get("is_valid")),
    }
    if "eph" in packet:
        out["eph"] = _finite_float("eph", packet.get("eph"))
    if "epv" in packet:
        out["epv"] = _finite_float("epv", packet.get("epv"))
    if "fix_type" in packet:
        fix_type = _uint("fix_type", packet.get("fix_type"))
        if fix_type > 3:
            raise ProtocolError("fix_type must be in [0, 3]")
        out["fix_type"] = fix_type
    if "time_utc" in packet:
        out["time_utc"] = _uint("time_utc", packet.get("time_utc"))
    return out


def validate_sensor_barometer_packet(packet: dict[str, Any]) -> dict[str, Any]:
    return {
        **_base_packet(packet, SENSOR_BAROMETER_PACKET),
        "altitude": _finite_float("altitude", packet.get("altitude")),
        "pressure": _finite_float("pressure", packet.get("pressure")),
        "qnh": _finite_float("qnh", packet.get("qnh")),
    }


def validate_sensor_magnetometer_packet(packet: dict[str, Any]) -> dict[str, Any]:
    out = {
        **_base_packet(packet, SENSOR_MAGNETOMETER_PACKET),
        "frame_id": str(packet.get("frame_id", "aircraft_frd")),
        "magnetic_field_body": _vector("magnetic_field_body", packet.get("magnetic_field_body"), 3),
    }
    if "magnetic_field_covariance" in packet:
        out["magnetic_field_covariance"] = _vector(
            "magnetic_field_covariance",
            packet.get("magnetic_field_covariance"),
            9,
        )
    return out


def validate_sensor_rangefinder_packet(packet: dict[str, Any]) -> dict[str, Any]:
    out = {
        **_base_packet(packet, SENSOR_RANGEFINDER_PACKET),
        "distance": _finite_float("distance", packet.get("distance")),
        "min_distance": _finite_float("min_distance", packet.get("min_distance")),
        "max_distance": _finite_float("max_distance", packet.get("max_distance")),
    }
    if "frame_id" in packet:
        out["frame_id"] = str(packet.get("frame_id"))
    if "relative_pose" in packet:
        out["relative_pose"] = _pose("relative_pose", packet.get("relative_pose"))
    return out


def validate_sensor_lidar_packet(packet: dict[str, Any]) -> dict[str, Any]:
    point_cloud = _float_array("point_cloud", packet.get("point_cloud"))
    if len(point_cloud) % 3 != 0:
        raise ProtocolError("point_cloud length must be a multiple of 3")
    out = {
        **_base_packet(packet, SENSOR_LIDAR_PACKET),
        "frame_id": str(packet.get("frame_id", "lidar_local_ned")),
        "point_cloud": point_cloud,
    }
    if "segmentation" in packet:
        out["segmentation"] = _int_array("segmentation", packet.get("segmentation"))
    if "pose" in packet:
        out["pose"] = _pose("pose", packet.get("pose"))
    return out


def validate_environment_packet(packet: dict[str, Any]) -> dict[str, Any]:
    source_policy = str(packet.get("source_policy", "configured"))
    if source_policy not in {"configured", "unknown"}:
        raise ProtocolError("source_policy must be configured or unknown")
    return {
        **_base_packet(packet, ENVIRONMENT_PACKET),
        "wind_ned_mps": _vector("wind_ned_mps", packet.get("wind_ned_mps"), 3),
        "source_policy": source_policy,
    }


def validate_collision_packet(packet: dict[str, Any]) -> dict[str, Any]:
    return {
        **_base_packet(packet, COLLISION_PACKET),
        "has_collided": _bool("has_collided", packet.get("has_collided")),
        "normal": _vector("normal", packet.get("normal"), 3),
        "impact_point": _vector("impact_point", packet.get("impact_point"), 3),
        "position": _vector("position", packet.get("position"), 3),
        "penetration_depth": _finite_float("penetration_depth", packet.get("penetration_depth")),
        "collision_count": _uint("collision_count", packet.get("collision_count")),
        "object_name": str(packet.get("object_name", "")),
        "object_id": int(packet.get("object_id", -1)),
    }


def validate_motor_state_packet(packet: dict[str, Any]) -> dict[str, Any]:
    raw_rotors = packet.get("rotors")
    if not isinstance(raw_rotors, (list, tuple)):
        raise ProtocolError("rotors must be an array")
    rotors: list[dict[str, float]] = []
    for idx, rotor in enumerate(raw_rotors):
        raw = _object(f"rotors[{idx}]", rotor)
        rotors.append(
            {
                "thrust": _finite_float(f"rotors[{idx}].thrust", raw.get("thrust")),
                "torque_scaler": _finite_float(
                    f"rotors[{idx}].torque_scaler",
                    raw.get("torque_scaler"),
                ),
                "speed": _finite_float(f"rotors[{idx}].speed", raw.get("speed")),
            }
        )
    out = {
        **_base_packet(packet, MOTOR_STATE_PACKET),
        "rotors": rotors,
    }
    if "native_timestamp" in packet:
        out["native_timestamp"] = _uint("native_timestamp", packet.get("native_timestamp"))
    return out
