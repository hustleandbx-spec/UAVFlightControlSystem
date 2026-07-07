import unittest

from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "ros2_ws" / "src" / "aircraft_udp_bridge"))

from aircraft_udp_bridge.protocol import (  # noqa: E402
    ProtocolError,
    decode_datagram,
    encode_packet,
    make_actuator_packet,
    make_control_packet,
    make_state_packet,
    validate_actuator_packet,
    validate_command_packet,
    validate_control_packet,
    validate_state_packet,
)


class ProtocolTest(unittest.TestCase):
    def test_state_roundtrip(self) -> None:
        packet = make_state_packet(
            timestamp=1.0,
            sequence=1,
            position=[1.0, 2.0, -3.0],
            orientation=[0.0, 0.0, 0.0, 1.0],
            linear_velocity=[0.1, 0.2, 0.3],
            angular_velocity=[0.01, 0.02, 0.03],
        )
        decoded = validate_state_packet(decode_datagram(encode_packet(packet)))
        self.assertEqual(decoded["packet_type"], "state")
        self.assertEqual(decoded["position"], [1.0, 2.0, -3.0])

    def test_control_roundtrip(self) -> None:
        packet = make_control_packet(
            timestamp=1.0,
            sequence=2,
            mode="rate",
            throttle=0.5,
            roll=0.1,
            pitch=0.2,
            yaw=0.3,
        )
        decoded = validate_control_packet(decode_datagram(encode_packet(packet)))
        self.assertEqual(decoded["packet_type"], "control")
        self.assertEqual(decoded["mode"], "rate")

    def test_actuator_roundtrip(self) -> None:
        packet = make_actuator_packet(
            timestamp=1.25,
            sequence=3,
            motor_cmd=[0.1, 0.2, 0.3, 0.4],
        )
        decoded = validate_actuator_packet(decode_datagram(encode_packet(packet)))
        self.assertEqual(decoded["packet_type"], "actuator")
        self.assertEqual(decoded["mode"], "motor")
        self.assertEqual(decoded["motor_cmd"], [0.1, 0.2, 0.3, 0.4])
        self.assertEqual(validate_command_packet(decoded), decoded)

    def test_reject_bad_throttle(self) -> None:
        with self.assertRaises(ProtocolError):
            make_control_packet(
                timestamp=1.0,
                sequence=2,
                mode="rate",
                throttle=1.2,
                roll=0.0,
                pitch=0.0,
                yaw=0.0,
            )

    def test_reject_bad_motor_command(self) -> None:
        with self.assertRaises(ProtocolError):
            make_actuator_packet(
                timestamp=1.0,
                sequence=4,
                motor_cmd=[0.0, 0.5, 1.2, 0.0],
            )


if __name__ == "__main__":
    unittest.main()
