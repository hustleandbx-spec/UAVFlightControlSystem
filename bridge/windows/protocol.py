"""Vendored UDP JSON protocol helpers for the Windows AirSim endpoint.

Authority: WSL ~/uavsingle_ros2_ws/src/aircraft_udp_bridge/aircraft_udp_bridge/protocol.py
This copy is vendored per architecture rule: "UDP schemas 权威在 WSL，Windows 仅 vendor".

Only the packet types needed by the endpoint are included here.
"""

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
CONTROL_MODES = {"rate", "attitude"}
ACTUATOR_MODES = {"motor"}
MAX_DATAGRAM_BYTES = 8192


class ProtocolError(ValueError):
    pass


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


def make_sensor_imu_packet(
    *,
    timestamp: float | None = None,
    sequence: int,
    source_id: int = 0,
    frame_id: str = "aircraft_frd",
    orientation: list[float] | tuple[float, float, float, float],
    angular_velocity: list[float] | tuple[float, float, float],
    linear_acceleration: list[float] | tuple[float, float, float],
) -> dict[str, Any]:
    packet = {
        "protocol_version": PROTOCOL_VERSION,
        "packet_type": SENSOR_IMU_PACKET,
        "timestamp": time.time() if timestamp is None else timestamp,
        "sequence": sequence,
        "source_id": source_id,
        "frame_id": frame_id,
        "orientation": list(orientation),
        "angular_velocity": list(angular_velocity),
        "linear_acceleration": list(linear_acceleration),
    }
    return validate_sensor_imu_packet(packet)


def make_sensor_gps_packet(
    *,
    timestamp: float | None = None,
    sequence: int,
    source_id: int = 0,
    latitude: float,
    longitude: float,
    altitude: float,
    velocity: list[float] | tuple[float, float, float],
    is_valid: bool = True,
) -> dict[str, Any]:
    packet = {
        "protocol_version": PROTOCOL_VERSION,
        "packet_type": SENSOR_GPS_PACKET,
        "timestamp": time.time() if timestamp is None else timestamp,
        "sequence": sequence,
        "source_id": source_id,
        "geo_point": {
            "latitude": latitude,
            "longitude": longitude,
            "altitude": altitude,
        },
        "velocity": list(velocity),
        "is_valid": is_valid,
    }
    return validate_sensor_gps_packet(packet)


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
