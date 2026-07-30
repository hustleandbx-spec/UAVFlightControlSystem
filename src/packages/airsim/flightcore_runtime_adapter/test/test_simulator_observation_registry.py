import json
import unittest
from pathlib import Path
import sys

import jsonschema
import yaml

PACKAGES_ROOT = Path(__file__).resolve().parents[2]
AIRCRAFT_BRIDGE_ROOT = PACKAGES_ROOT / "aircraft_udp_bridge"
ADAPTER_ROOT = PACKAGES_ROOT / "flightcore_runtime_adapter"
SCHEMAS = AIRCRAFT_BRIDGE_ROOT / "schemas"
sys.path.insert(0, str(AIRCRAFT_BRIDGE_ROOT))
sys.path.insert(0, str(ADAPTER_ROOT))

from aircraft_udp_bridge.protocol import validate_observation_packet  # noqa: E402
from flightcore_runtime_adapter.observation_router import (  # noqa: E402
    CONTRACT_CANDIDATE,
    SCENARIO,
    TRACE,
    UAV_CONTRACT,
    UNSUPPORTED,
    route_observation_entry,
)


REGISTRY_PATH = ADAPTER_ROOT / "config" / "simulator_observation_registry.yaml"

SAMPLES = {
    "state": {
        "protocol_version": 1,
        "packet_type": "state",
        "timestamp": 1.0,
        "sequence": 1,
        "position": [0.0, 0.0, -1.0],
        "orientation": [0.0, 0.0, 0.0, 1.0],
        "linear_velocity": [0.0, 0.0, 0.0],
        "angular_velocity": [0.0, 0.0, 0.0],
    },
    "sensor_imu": {
        "protocol_version": 1,
        "packet_type": "sensor_imu",
        "timestamp": 1.0,
        "sequence": 2,
        "orientation": [0.0, 0.0, 0.0, 1.0],
        "angular_velocity": [0.1, 0.2, 0.3],
        "linear_acceleration": [0.0, 0.0, -9.80665],
    },
    "sensor_gps": {
        "protocol_version": 1,
        "packet_type": "sensor_gps",
        "timestamp": 1.0,
        "sequence": 3,
        "geo_point": {
            "latitude": 47.641468,
            "longitude": -122.140165,
            "altitude": 122.0,
        },
        "velocity": [0.0, 0.0, 0.0],
        "eph": 0.2,
        "epv": 0.3,
        "fix_type": 3,
        "time_utc": 1710000000000000000,
        "is_valid": True,
    },
    "sensor_barometer": {
        "protocol_version": 1,
        "packet_type": "sensor_barometer",
        "timestamp": 1.0,
        "sequence": 4,
        "altitude": 120.0,
        "pressure": 101325.0,
        "qnh": 1013.25,
    },
    "sensor_magnetometer": {
        "protocol_version": 1,
        "packet_type": "sensor_magnetometer",
        "timestamp": 1.0,
        "sequence": 5,
        "magnetic_field_body": [0.2, 0.0, 0.4],
        "magnetic_field_covariance": [0.0] * 9,
    },
    "sensor_rangefinder": {
        "protocol_version": 1,
        "packet_type": "sensor_rangefinder",
        "timestamp": 1.0,
        "sequence": 6,
        "distance": 3.0,
        "min_distance": 0.2,
        "max_distance": 40.0,
        "relative_pose": {
            "position": [0.0, 0.0, 0.0],
            "orientation": [0.0, 0.0, 0.0, 1.0],
        },
    },
    "sensor_lidar": {
        "protocol_version": 1,
        "packet_type": "sensor_lidar",
        "timestamp": 1.0,
        "sequence": 7,
        "point_cloud": [0.0, 0.0, 0.0, 1.0, 0.0, 0.0],
        "segmentation": [0, 1],
        "pose": {
            "position": [0.0, 0.0, 0.0],
            "orientation": [0.0, 0.0, 0.0, 1.0],
        },
    },
    "environment": {
        "protocol_version": 1,
        "packet_type": "environment",
        "timestamp": 1.0,
        "sequence": 8,
        "wind_ned_mps": [1.0, 0.0, 0.0],
        "source_policy": "configured",
    },
    "collision": {
        "protocol_version": 1,
        "packet_type": "collision",
        "timestamp": 1.0,
        "sequence": 9,
        "has_collided": True,
        "normal": [0.0, 0.0, -1.0],
        "impact_point": [1.0, 2.0, -0.1],
        "position": [1.0, 2.0, -0.1],
        "penetration_depth": 0.02,
        "collision_count": 1,
        "object_name": "ground",
        "object_id": 42,
    },
    "motor_state": {
        "protocol_version": 1,
        "packet_type": "motor_state",
        "timestamp": 1.0,
        "sequence": 10,
        "native_timestamp": 1710000000000000000,
        "rotors": [
            {"thrust": 1.0, "torque_scaler": 0.01, "speed": 400.0},
            {"thrust": 1.1, "torque_scaler": 0.01, "speed": 410.0},
        ],
    },
}


def load_registry() -> dict:
    with REGISTRY_PATH.open("r", encoding="utf-8") as handle:
        return yaml.safe_load(handle)


class SimulatorObservationRegistryTest(unittest.TestCase):
    def test_registry_matches_registry_schema(self) -> None:
        registry = load_registry()
        schema = json.loads((SCHEMAS / "simulator_observation_registry.schema.json").read_text())
        jsonschema.Draft202012Validator.check_schema(schema)
        jsonschema.validate(registry, schema)

    def test_declared_packet_types_have_schema_and_validate_sample(self) -> None:
        registry = load_registry()
        packet_types = {
            item["packet_type"]
            for item in registry["observations"]
            if "packet_type" in item
        }

        for packet_type in sorted(packet_types):
            schema_path = SCHEMAS / f"{packet_type}.schema.json"
            self.assertTrue(schema_path.exists(), packet_type)
            schema = json.loads(schema_path.read_text())
            jsonschema.Draft202012Validator.check_schema(schema)
            sample = SAMPLES[packet_type]
            jsonschema.validate(sample, schema)
            self.assertEqual(validate_observation_packet(sample)["packet_type"], packet_type)

    def test_router_blocks_truth_environment_events_and_motor_feedback_from_uav(self) -> None:
        registry = load_registry()
        routes = {item["id"]: route_observation_entry(item) for item in registry["observations"]}

        self.assertIn(UAV_CONTRACT, routes["airsim_imu"].outputs)
        self.assertIn(UAV_CONTRACT, routes["airsim_gps"].outputs)
        self.assertIn(CONTRACT_CANDIDATE, routes["airsim_barometer"].outputs)

        blocked_ids = [
            "airsim_truth_state",
            "airsim_collision",
            "airsim_wind",
            "airsim_motor_state",
            "isaac_contact",
            "pegasus_state",
        ]
        for item_id in blocked_ids:
            self.assertNotIn(UAV_CONTRACT, routes[item_id].outputs, item_id)

        self.assertIn(TRACE, routes["airsim_truth_state"].outputs)
        self.assertIn(SCENARIO, routes["airsim_collision"].outputs)
        self.assertIn(TRACE, routes["airsim_motor_state"].outputs)
        self.assertEqual(routes["airsim_battery"].outputs, (UNSUPPORTED,))


if __name__ == "__main__":
    unittest.main()
