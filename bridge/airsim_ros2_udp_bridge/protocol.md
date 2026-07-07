# Aircraft UDP JSON Protocol v1

One UDP datagram contains exactly one UTF-8 JSON object. Messages are small and
stateless; packet loss is handled by publishing the newest received state.

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
Do not convert this packet to throttle/roll/pitch/yaw as the primary closed-loop
path.

The protocol is simulator-neutral. AirSim, Gazebo, Isaac Sim, or another backend
must adapt its native API to this schema at the endpoint boundary.
