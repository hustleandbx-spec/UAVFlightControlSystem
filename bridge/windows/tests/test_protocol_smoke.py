"""Protocol packet smoke tests (P1).

These tests verify the protocol encode/decode roundtrip WITHOUT needing
WSL ROS2, MATLAB, or AirSim.  They can run on Windows standalone.

Run:
  python -m pytest tests/test_protocol_smoke.py -v
  python tests/test_protocol_smoke.py  (unittest runner)
"""

import json
import math
import struct
import time
import unittest
from pathlib import Path
from typing import Any

import sys

# Import vendored protocol from bridge/windows/.
PROTOCOL_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROTOCOL_DIR))

from protocol import (  # type: ignore
    ACTUATOR_PACKET,
    CONTROL_PACKET,
    SENSOR_GPS_PACKET,
    SENSOR_IMU_PACKET,
    STATE_PACKET,
    ProtocolError,
    decode_datagram,
    encode_packet,
    make_actuator_packet,
    make_control_packet,
    make_sensor_gps_packet,
    make_sensor_imu_packet,
    make_state_packet,
    validate_actuator_packet,
    validate_command_packet,
    validate_control_packet,
    validate_sensor_gps_packet,
    validate_sensor_imu_packet,
    validate_state_packet,
)


class TestProtocolRoundtrip(unittest.TestCase):
    """Verify all packet types encode/decode roundtrip correctly."""

    def test_state_roundtrip(self) -> None:
        pkt = make_state_packet(
            timestamp=1000.0,
            sequence=1,
            position=[1.0, 2.0, -3.0],
            orientation=[0.0, 0.0, 0.0, 1.0],
            linear_velocity=[0.1, 0.2, 0.3],
            angular_velocity=[0.01, 0.02, 0.03],
        )
        decoded = validate_state_packet(decode_datagram(encode_packet(pkt)))
        self.assertEqual(decoded["packet_type"], STATE_PACKET)
        self.assertEqual(decoded["position"], [1.0, 2.0, -3.0])
        self.assertEqual(decoded["orientation"], [0.0, 0.0, 0.0, 1.0])
        self.assertEqual(decoded["sequence"], 1)

    def test_sensor_imu_roundtrip(self) -> None:
        pkt = make_sensor_imu_packet(
            timestamp=1000.0,
            sequence=5,
            source_id=0,
            frame_id="aircraft_frd",
            orientation=[0.0, 0.0, 0.0, 1.0],
            angular_velocity=[0.01, 0.02, 0.03],
            linear_acceleration=[0.0, 0.0, -9.80665],
        )
        decoded = validate_sensor_imu_packet(decode_datagram(encode_packet(pkt)))
        self.assertEqual(decoded["packet_type"], SENSOR_IMU_PACKET)
        self.assertEqual(decoded["linear_acceleration"], [0.0, 0.0, -9.80665])
        self.assertEqual(decoded["angular_velocity"], [0.01, 0.02, 0.03])

    def test_sensor_gps_roundtrip(self) -> None:
        pkt = make_sensor_gps_packet(
            timestamp=1000.0,
            sequence=10,
            source_id=0,
            latitude=47.641468,
            longitude=-122.140165,
            altitude=122.0,
            velocity=[0.0, 0.0, 0.0],
            is_valid=True,
        )
        decoded = validate_sensor_gps_packet(decode_datagram(encode_packet(pkt)))
        self.assertEqual(decoded["packet_type"], SENSOR_GPS_PACKET)
        self.assertEqual(decoded["geo_point"]["latitude"], 47.641468)
        self.assertTrue(decoded["is_valid"])

    def test_actuator_roundtrip(self) -> None:
        pkt = make_actuator_packet(
            timestamp=1000.25,
            sequence=3,
            motor_cmd=[0.45, 0.45, 0.45, 0.45],
        )
        decoded = validate_actuator_packet(decode_datagram(encode_packet(pkt)))
        self.assertEqual(decoded["packet_type"], ACTUATOR_PACKET)
        self.assertEqual(decoded["mode"], "motor")
        self.assertEqual(decoded["motor_cmd"], [0.45, 0.45, 0.45, 0.45])

    def test_actuator_rejects_overflow(self) -> None:
        with self.assertRaises(ProtocolError):
            make_actuator_packet(
                timestamp=1.0,
                sequence=4,
                motor_cmd=[0.0, 0.5, 1.2, 0.0],
            )

    def test_control_roundtrip(self) -> None:
        pkt = make_control_packet(
            timestamp=1.0,
            sequence=2,
            mode="rate",
            throttle=0.5,
            roll=0.1,
            pitch=0.2,
            yaw=0.3,
        )
        decoded = validate_control_packet(decode_datagram(encode_packet(pkt)))
        self.assertEqual(decoded["mode"], "rate")

    def test_validate_command_accepts_actuator(self) -> None:
        pkt = make_actuator_packet(sequence=1, motor_cmd=[0.1, 0.2, 0.3, 0.4])
        self.assertEqual(validate_command_packet(pkt)["packet_type"], ACTUATOR_PACKET)

    def test_validate_command_accepts_control(self) -> None:
        pkt = make_control_packet(sequence=1, mode="rate", throttle=0.5, roll=0, pitch=0, yaw=0)
        self.assertEqual(validate_command_packet(pkt)["packet_type"], CONTROL_PACKET)


class TestPacketConstruction(unittest.TestCase):
    """Verify protocol packet constructors produce valid packets."""

    def test_state_packet(self) -> None:
        """Verify make_state_packet produces valid output."""
        start = time.time()
        pkt = make_state_packet(
            sequence=1,
            position=[0.0, 1.0, -2.0],
            orientation=[0.0, 0.0, 0.0, 1.0],
            linear_velocity=[0.2, 0.0, 0.0],
            angular_velocity=[0.0, 0.0, 0.0],
        )
        self.assertEqual(pkt["packet_type"], STATE_PACKET)
        self.assertGreater(pkt["timestamp"], start - 1)

    def test_imu_packet(self) -> None:
        """Verify make_sensor_imu_packet produces valid output."""
        pkt = make_sensor_imu_packet(
            sequence=1,
            source_id=0,
            orientation=[0.0, 0.0, 0.0, 1.0],
            angular_velocity=[0.0, 0.0, 0.0],
            linear_acceleration=[0.0, 0.0, -9.80665],
        )
        validate_sensor_imu_packet(pkt)  # should not raise

    def test_gps_packet(self) -> None:
        """Verify make_sensor_gps_packet produces valid output."""
        pkt = make_sensor_gps_packet(
            sequence=1,
            source_id=0,
            latitude=47.641468,
            longitude=-122.140165,
            altitude=122.0,
            velocity=[0.2, 0.0, 0.0],
            is_valid=True,
        )
        validate_sensor_gps_packet(pkt)  # should not raise

    def test_all_packet_types_endpoint_tick(self) -> None:
        """Verify all three sensor packet types encode correctly in one tick."""
        ts = time.time()
        packets = [
            make_state_packet(sequence=1, position=[0, 0, -2], orientation=[0, 0, 0, 1],
                              linear_velocity=[0, 0, 0], angular_velocity=[0, 0, 0]),
            make_sensor_imu_packet(sequence=1, source_id=0, orientation=[0, 0, 0, 1],
                                   angular_velocity=[0, 0, 0], linear_acceleration=[0, 0, -9.80665]),
            make_sensor_gps_packet(sequence=1, source_id=0, latitude=47.641468,
                                   longitude=-122.140165, altitude=122.0,
                                   velocity=[0, 0, 0], is_valid=True),
        ]
        for pkt in packets:
            encoded = encode_packet(pkt)
            decoded = decode_datagram(encoded)
            self.assertEqual(decoded["protocol_version"], 1)
            self.assertIn(decoded["packet_type"], {STATE_PACKET, SENSOR_IMU_PACKET, SENSOR_GPS_PACKET})

    def test_actuator_udp_encoding_size(self) -> None:
        """Verify actuator packet fits in a single UDP datagram."""
        pkt = make_actuator_packet(sequence=1000, motor_cmd=[0.5, 0.5, 0.5, 0.5])
        encoded = encode_packet(pkt)
        self.assertLess(len(encoded), 512)  # well under MTU


class TestPacketSequences(unittest.TestCase):
    """Verify sequence numbering works correctly."""

    def test_state_imu_gps_independent_sequences(self) -> None:
        """State, IMU, and GPS should have independent sequence counters."""
        state_pkt = make_state_packet(sequence=50, position=[0, 0, -2],
                                       orientation=[0, 0, 0, 1],
                                       linear_velocity=[0, 0, 0],
                                       angular_velocity=[0, 0, 0])
        imu_pkt = make_sensor_imu_packet(sequence=50, source_id=0,
                                          orientation=[0, 0, 0, 1],
                                          angular_velocity=[0, 0, 0],
                                          linear_acceleration=[0, 0, -9.80665])
        gps_pkt = make_sensor_gps_packet(sequence=5, source_id=0,
                                          latitude=47.64, longitude=-122.14,
                                          altitude=122.0, velocity=[0, 0, 0])
        # All valid independently
        validate_state_packet(state_pkt)
        validate_sensor_imu_packet(imu_pkt)
        validate_sensor_gps_packet(gps_pkt)

    def test_actuator_mode_must_be_motor(self) -> None:
        with self.assertRaises(ProtocolError):
            make_actuator_packet(sequence=1, motor_cmd=[0.1, 0.2, 0.3, 0.4], mode="throttle")


if __name__ == "__main__":
    unittest.main()
