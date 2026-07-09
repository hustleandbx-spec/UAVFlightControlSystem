import math
import unittest
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
WSL_SRC = ROOT / "wsl" / "packages"
sys.path.insert(0, str(WSL_SRC / "aircraft_udp_bridge"))
sys.path.insert(0, str(WSL_SRC / "flightcore_runtime_adapter"))

try:
    from flightcore_runtime_adapter.mapping import (  # noqa: E402
        EARTH_RADIUS_M,
        LocalOrigin,
        Stamp,
        esc_cmd_to_actuator_packet,
        flight_cmd_to_contract,
        imu_to_contract,
        local_ned_to_lla,
        odometry_to_debug_gps_contract,
    )
except ModuleNotFoundError as exc:  # pragma: no cover - depends on WSL checkout.
    raise unittest.SkipTest(
        "flightcore_runtime_adapter is maintained in the WSL native workspace; "
        "Windows repo no longer vendors this package."
    ) from exc


class RuntimeAdapterMappingTest(unittest.TestCase):
    def test_imu_mapping_uses_stamp_and_vectors(self) -> None:
        out = imu_to_contract(
            stamp=Stamp(sec=10, nanosec=250_000_000),
            now_sec=99.0,
            sequence=7,
            source_id=2,
            valid=True,
            linear_acceleration=[1.0, 2.0, -9.8],
            angular_velocity=[0.1, 0.2, 0.3],
        )

        self.assertAlmostEqual(out["timestamp_sec"], 10.25)
        self.assertEqual(out["sequence"], 7)
        self.assertEqual(out["source_id"], 2)
        self.assertTrue(out["valid"])
        self.assertEqual(out["accel_mps2"], [1.0, 2.0, -9.8])
        self.assertEqual(out["gyro_radps"], [0.1, 0.2, 0.3])

    def test_imu_mapping_falls_back_to_now_for_zero_stamp(self) -> None:
        out = imu_to_contract(
            stamp=Stamp(),
            now_sec=12.5,
            sequence=8,
            source_id=1,
            valid=False,
            linear_acceleration=[0.0, 0.0, 0.0],
            angular_velocity=[0.0, 0.0, 0.0],
        )

        self.assertAlmostEqual(out["timestamp_sec"], 12.5)
        self.assertFalse(out["valid"])

    def test_local_ned_to_lla_approximation(self) -> None:
        origin = LocalOrigin(lat_deg=45.0, lon_deg=7.0, alt_m=100.0)
        north = 10.0
        east = 20.0
        down = -3.0

        lat, lon, alt = local_ned_to_lla([north, east, down], origin)

        self.assertAlmostEqual(lat, origin.lat_deg + math.degrees(north / EARTH_RADIUS_M))
        self.assertAlmostEqual(
            lon,
            origin.lon_deg
            + math.degrees(east / (EARTH_RADIUS_M * math.cos(math.radians(origin.lat_deg)))),
        )
        self.assertAlmostEqual(alt, 103.0)

    def test_state_debug_gps_mapping_keeps_velocity(self) -> None:
        origin = LocalOrigin(lat_deg=0.0, lon_deg=0.0, alt_m=50.0)
        out = odometry_to_debug_gps_contract(
            stamp=Stamp(sec=20, nanosec=0),
            now_sec=99.0,
            sequence=3,
            source_id=1,
            valid=True,
            position_ned_m=[1.0, 2.0, -4.0],
            velocity_ned_mps=[0.4, 0.5, 0.6],
            local_origin=origin,
        )

        self.assertTrue(out["valid"])
        self.assertEqual(out["velocity_ned_mps"], [0.4, 0.5, 0.6])
        self.assertAlmostEqual(out["alt_m"], 54.0)

    def test_default_flight_command_mapping(self) -> None:
        out = flight_cmd_to_contract(
            stamp=Stamp(),
            now_sec=30.0,
            sequence=11,
            source_id=1,
            valid=True,
            mode=1,
            position_ned_sp_m=[0.0, 0.0, -2.0],
            velocity_ned_sp_mps=[0.0, 0.0, 0.0],
            yaw_sp_rad=0.0,
        )

        self.assertEqual(out["mode"], 1)
        self.assertEqual(out["position_ned_sp_m"], [0.0, 0.0, -2.0])
        self.assertAlmostEqual(out["timestamp_sec"], 30.0)

    def test_esc_cmd_to_actuator_packet(self) -> None:
        packet = esc_cmd_to_actuator_packet(
            timestamp_sec=42.0,
            sequence=12,
            motor_cmd=[0.11, 0.22, 0.33, 0.44],
        )

        self.assertEqual(packet["packet_type"], "actuator")
        self.assertEqual(packet["mode"], "motor")
        self.assertEqual(packet["motor_cmd"], [0.11, 0.22, 0.33, 0.44])


if __name__ == "__main__":
    unittest.main()
