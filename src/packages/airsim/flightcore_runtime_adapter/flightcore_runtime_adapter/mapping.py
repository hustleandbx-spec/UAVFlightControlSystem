from __future__ import annotations

import math
from dataclasses import dataclass
from typing import Sequence

from aircraft_udp_bridge.protocol import make_actuator_packet

EARTH_RADIUS_M = 6_378_137.0


@dataclass(frozen=True)
class Stamp:
    sec: int = 0
    nanosec: int = 0


@dataclass(frozen=True)
class LocalOrigin:
    lat_deg: float
    lon_deg: float
    alt_m: float


def stamp_to_seconds(stamp: Stamp, fallback_sec: float) -> float:
    value = float(stamp.sec) + float(stamp.nanosec) * 1.0e-9
    return value if value > 0.0 else float(fallback_sec)


def make_stamp(timestamp_sec: float) -> Stamp:
    sec = int(timestamp_sec)
    nanosec = int((timestamp_sec - sec) * 1_000_000_000)
    return Stamp(sec=sec, nanosec=max(0, min(nanosec, 999_999_999)))


def vector3(values: Sequence[float], name: str) -> list[float]:
    if len(values) != 3:
        raise ValueError(f"{name} must have 3 elements")
    out = [float(item) for item in values]
    if not all(math.isfinite(item) for item in out):
        raise ValueError(f"{name} must contain finite values")
    return out


def vector4(values: Sequence[float], name: str) -> list[float]:
    if len(values) != 4:
        raise ValueError(f"{name} must have 4 elements")
    out = [float(item) for item in values]
    if not all(math.isfinite(item) for item in out):
        raise ValueError(f"{name} must contain finite values")
    return out


def imu_to_contract(
    *,
    stamp: Stamp,
    now_sec: float,
    sequence: int,
    source_id: int,
    valid: bool,
    linear_acceleration: Sequence[float],
    angular_velocity: Sequence[float],
) -> dict[str, object]:
    timestamp_sec = stamp_to_seconds(stamp, now_sec)
    return {
        "stamp": make_stamp(timestamp_sec),
        "timestamp_sec": timestamp_sec,
        "sequence": int(sequence),
        "source_id": int(source_id),
        "valid": bool(valid),
        "accel_mps2": vector3(linear_acceleration, "linear_acceleration"),
        "gyro_radps": vector3(angular_velocity, "angular_velocity"),
    }


def navsat_to_contract(
    *,
    stamp: Stamp,
    now_sec: float,
    sequence: int,
    source_id: int,
    valid: bool,
    lat_deg: float,
    lon_deg: float,
    alt_m: float,
    velocity_ned_mps: Sequence[float],
) -> dict[str, object]:
    timestamp_sec = stamp_to_seconds(stamp, now_sec)
    fields = {
        "lat_deg": float(lat_deg),
        "lon_deg": float(lon_deg),
        "alt_m": float(alt_m),
    }
    if not all(math.isfinite(value) for value in fields.values()):
        raise ValueError("gps position fields must be finite")
    return {
        "stamp": make_stamp(timestamp_sec),
        "timestamp_sec": timestamp_sec,
        "sequence": int(sequence),
        "source_id": int(source_id),
        "valid": bool(valid),
        **fields,
        "velocity_ned_mps": vector3(velocity_ned_mps, "velocity_ned_mps"),
    }


def local_ned_to_lla(position_ned_m: Sequence[float], origin: LocalOrigin) -> tuple[float, float, float]:
    north_m, east_m, down_m = vector3(position_ned_m, "position_ned_m")
    lat0_rad = math.radians(origin.lat_deg)
    cos_lat0 = math.cos(lat0_rad)
    if abs(cos_lat0) < 1.0e-9:
        raise ValueError("local origin latitude is too close to a pole")
    lat_deg = origin.lat_deg + math.degrees(north_m / EARTH_RADIUS_M)
    lon_deg = origin.lon_deg + math.degrees(east_m / (EARTH_RADIUS_M * cos_lat0))
    alt_m = origin.alt_m - down_m
    return lat_deg, lon_deg, alt_m


def odometry_to_debug_gps_contract(
    *,
    stamp: Stamp,
    now_sec: float,
    sequence: int,
    source_id: int,
    valid: bool,
    position_ned_m: Sequence[float],
    velocity_ned_mps: Sequence[float],
    local_origin: LocalOrigin,
) -> dict[str, object]:
    lat_deg, lon_deg, alt_m = local_ned_to_lla(position_ned_m, local_origin)
    return navsat_to_contract(
        stamp=stamp,
        now_sec=now_sec,
        sequence=sequence,
        source_id=source_id,
        valid=valid,
        lat_deg=lat_deg,
        lon_deg=lon_deg,
        alt_m=alt_m,
        velocity_ned_mps=velocity_ned_mps,
    )


def flight_cmd_to_contract(
    *,
    stamp: Stamp,
    now_sec: float,
    sequence: int,
    source_id: int,
    valid: bool,
    mode: int,
    position_ned_sp_m: Sequence[float],
    velocity_ned_sp_mps: Sequence[float],
    yaw_sp_rad: float,
) -> dict[str, object]:
    timestamp_sec = stamp_to_seconds(stamp, now_sec)
    yaw = float(yaw_sp_rad)
    if not math.isfinite(yaw):
        raise ValueError("yaw_sp_rad must be finite")
    return {
        "stamp": make_stamp(timestamp_sec),
        "timestamp_sec": timestamp_sec,
        "sequence": int(sequence),
        "source_id": int(source_id),
        "valid": bool(valid),
        "mode": int(mode),
        "position_ned_sp_m": vector3(position_ned_sp_m, "position_ned_sp_m"),
        "velocity_ned_sp_mps": vector3(velocity_ned_sp_mps, "velocity_ned_sp_mps"),
        "yaw_sp_rad": yaw,
    }


def esc_cmd_to_actuator_packet(
    *,
    timestamp_sec: float,
    sequence: int,
    motor_cmd: Sequence[float],
) -> dict[str, object]:
    return make_actuator_packet(
        timestamp=timestamp_sec,
        sequence=int(sequence),
        motor_cmd=vector4(motor_cmd, "motor_cmd"),
    )
