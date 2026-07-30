# Aircraft UDP JSON Protocol v1

One UDP datagram contains exactly one UTF-8 JSON object. Messages are small and
stateless; packet loss is handled by publishing the newest received state.

## Packet Families

Packet types are versioned by `protocol_version: 1` and by one JSON schema file
per packet type under `schemas/`.

Observation packets flow from simulator endpoint to WSL2 bridge/adapter:

| packet_type | Primary target | Notes |
|---|---|---|
| `state` | `/aircraft/truth/state` legacy alias `/aircraft/state` | Truth/runtime state only |
| `sensor_imu` | `/aircraft/sensors/imu` | May map to current `/uav/sensors/imu` |
| `sensor_gps` | `/aircraft/sensors/gps` | May map to current `/uav/sensors/gps` |
| `sensor_barometer` | `/aircraft/sensors/barometer` | Future contract candidate |
| `sensor_magnetometer` | `/aircraft/sensors/magnetometer` | Future contract candidate |
| `sensor_rangefinder` | `/aircraft/sensors/rangefinder` | Future contract candidate |
| `sensor_lidar` | `/aircraft/sensors/lidar` | Trace/perception path only for now |
| `environment` | `/aircraft/environment/wind` | Scenario/internal state, not FlightCore input |
| `collision` | `/aircraft/events/collision` | Scenario/fault and trace only |
| `motor_state` | `/aircraft/actuators/motor_state` | Simulator feedback, trace-only |

Command packets flow from WSL2 runtime adapter/bridge to simulator endpoint:

| packet_type | Purpose |
|---|---|
| `actuator` | FlightCore normalized motor setpoints |
| `control` | Legacy rate/attitude debug command |

## State Packet

Direction: simulator endpoint -> WSL2 bridge.

```json
{
  "protocol_version": 1,
  "packet_type": "state",
  "timestamp": 1710000000.0,
  "sequence": 1,
  "frame_id": "map_ned",
  "child_frame_id": "aircraft_frd",
  "position": [0.0, 0.0, -2.0],
  "orientation": [0.0, 0.0, 0.0, 1.0],
  "linear_velocity": [0.0, 0.0, 0.0],
  "angular_velocity": [0.0, 0.0, 0.0]
}
```

Field semantics:

| Field | Meaning |
|---|---|
| `timestamp` | Sender wall-clock seconds, Unix epoch |
| `sequence` | Monotonic unsigned integer, wraps allowed |
| `frame_id` | Parent frame for position and linear velocity |
| `child_frame_id` | Body frame for angular velocity |
| `position` | `[x, y, z]` meters in local NED by default |
| `orientation` | `[qx, qy, qz, qw]` quaternion |
| `linear_velocity` | `[vx, vy, vz]` m/s in `frame_id` |
| `angular_velocity` | `[wx, wy, wz]` rad/s in `child_frame_id` |

This packet remains for compatibility with the first bridge. New work should
treat it as `/aircraft/truth/state`, not as a FlightCore sensor.

## Sensor IMU Packet

Direction: simulator endpoint -> WSL2 bridge/adapter.

```json
{
  "protocol_version": 1,
  "packet_type": "sensor_imu",
  "timestamp": 1710000000.0,
  "sequence": 2,
  "source_id": 1,
  "frame_id": "aircraft_frd",
  "orientation": [0.0, 0.0, 0.0, 1.0],
  "angular_velocity": [0.0, 0.0, 0.0],
  "linear_acceleration": [0.0, 0.0, -9.80665]
}
```

Native evidence examples:

- AirSim `ImuBase::Output::{time_stamp, orientation, angular_velocity, linear_acceleration}`
- Gazebo IMU message fields `orientation`, `angular_velocity`, `linear_acceleration`
- Isaac IMU frame fields `lin_acc`, `ang_vel`, `orientation`, `time`

## Sensor GPS Packet

Direction: simulator endpoint -> WSL2 bridge/adapter.

```json
{
  "protocol_version": 1,
  "packet_type": "sensor_gps",
  "timestamp": 1710000000.0,
  "sequence": 3,
  "source_id": 1,
  "geo_point": {
    "latitude": 47.641468,
    "longitude": -122.140165,
    "altitude": 122.0
  },
  "velocity": [0.0, 0.0, 0.0],
  "eph": 0.2,
  "epv": 0.3,
  "fix_type": 3,
  "time_utc": 1710000000000000000,
  "is_valid": true
}
```

Native evidence example: AirSim
`GpsBase::Output::{time_stamp, gnss, is_valid}` and
`GnssReport::{geo_point, eph, epv, velocity, fix_type, time_utc}`.

## Additional Sensor Observation Packets

The following packet types have schemas but are not yet mapped into current
`/uav/*` contracts:

| packet_type | Key native fields | Route |
|---|---|---|
| `sensor_barometer` | `altitude`, `pressure`, `qnh` | future sensor candidate |
| `sensor_magnetometer` | `magnetic_field_body`, `magnetic_field_covariance` | future sensor candidate |
| `sensor_rangefinder` | `distance`, `min_distance`, `max_distance`, `relative_pose` | future sensor candidate |
| `sensor_lidar` | `point_cloud`, `pose`, `segmentation` | trace/perception |

Do not add `/uav/*` topics for these until `InterfaceContract` and tests are
changed first.

## Environment Packet

Direction: simulator endpoint -> WSL2 bridge/adapter.

```json
{
  "protocol_version": 1,
  "packet_type": "environment",
  "timestamp": 1710000000.0,
  "sequence": 4,
  "wind_ned_mps": [1.0, 0.0, 0.0],
  "source_policy": "configured"
}
```

AirSim evidence currently confirms `simSetWind(Vector3r)` as a world-frame NED
m/s setting. It does not prove a readback observation API. Treat this packet as
scenario/internal state unless a simulator-specific measured wind source is
documented later.

## Collision Packet

Direction: simulator endpoint -> WSL2 bridge/adapter.

```json
{
  "protocol_version": 1,
  "packet_type": "collision",
  "timestamp": 1710000000.0,
  "sequence": 5,
  "has_collided": true,
  "normal": [0.0, 0.0, -1.0],
  "impact_point": [1.0, 2.0, -0.1],
  "position": [1.0, 2.0, -0.1],
  "penetration_depth": 0.02,
  "collision_count": 1,
  "object_name": "ground",
  "object_id": 42
}
```

Collision/contact data routes to scenario/fault handling and trace. It must not
enter FlightCore as a control-loop sensor.

## Motor State Packet

Direction: simulator endpoint -> WSL2 bridge/adapter.

```json
{
  "protocol_version": 1,
  "packet_type": "motor_state",
  "timestamp": 1710000000.0,
  "sequence": 6,
  "native_timestamp": 1710000000000000000,
  "rotors": [
    { "thrust": 1.0, "torque_scaler": 0.01, "speed": 400.0 }
  ]
}
```

This is simulator actuator feedback. It is useful for trace/evaluation and must
not be confused with FlightCore normalized motor setpoints.

## Control Packet

Direction: WSL2 bridge -> simulator endpoint.

```json
{
  "protocol_version": 1,
  "packet_type": "control",
  "timestamp": 1710000000.0,
  "sequence": 1,
  "mode": "rate",
  "throttle": 0.45,
  "roll": 0.0,
  "pitch": 0.0,
  "yaw": 0.0
}
```

Field semantics:

| Field | Meaning |
|---|---|
| `mode` | `rate` or `attitude` |
| `throttle` | Normalized command, `[0.0, 1.0]` |
| `roll` | Rate mode: roll rate rad/s. Attitude mode: roll angle rad |
| `pitch` | Rate mode: pitch rate rad/s. Attitude mode: pitch angle rad |
| `yaw` | Rate mode: yaw rate rad/s. Attitude mode: yaw angle rad |

This packet is retained for rate/attitude debug and mock testing. It is not the
FlightCore closed-loop main path.

## Actuator Packet

Direction: WSL2 runtime adapter -> simulator endpoint.

```json
{
  "protocol_version": 1,
  "packet_type": "actuator",
  "timestamp": 1710000000.0,
  "sequence": 1,
  "mode": "motor",
  "motor_cmd": [0.45, 0.45, 0.45, 0.45]
}
```

Field semantics:

| Field | Meaning |
|---|---|
| `mode` | `motor` |
| `motor_cmd` | Four normalized motor commands in FlightCore order, `[0.0, 1.0]` |

The endpoint maps `motor_cmd` into its native motor order through configuration.
Semantically this packet is `MotorSetpoint` / `ActuatorMotors`: normalized
FlightCore motor setpoints, not PWM, DShot, RPM, or real thrust. Do not convert
this packet to throttle/roll/pitch/yaw as the primary closed-loop path.

The protocol is simulator-neutral. AirSim, Gazebo, Isaac Sim, or another backend
must adapt its native API to this schema at the endpoint boundary.
