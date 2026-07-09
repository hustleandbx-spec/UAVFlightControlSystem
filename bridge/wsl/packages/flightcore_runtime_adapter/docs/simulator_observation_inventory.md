# Simulator Observation Inventory

> Baseline date: 2026-07-06
> Scope: phase-1 evidence-backed runtime observation ingress for AirSim, Gazebo,
> Isaac Sim, and Pegasus.

## Purpose

Phase 1 creates a place for simulator-observable quantities to enter the
Runtime Adapter without expanding the FlightCore product contract.

```text
simulator endpoint
  -> UDP observation packet family
  -> /aircraft/* Runtime Observation Layer
  -> Observation Router
      A. /uav/* FlightCore product contract
      B. /trace/* ExperimentTrace / logs / evaluation
      C. /scenario/* or adapter-internal scenario/fault state
```

This is intentionally not a direct FlightCore input expansion. Truth state,
real wind, collision events, real thrust, rendered images, and simulator-internal
state remain outside the FlightCore control loop unless a later product contract
change explicitly promotes a real vehicle sensor or actuator setpoint.

## Evidence Rule

Each candidate signal in
`bridge/airsim_ros2_udp_bridge/simulator_observation_registry.yaml` carries:

- `evidence_url_or_file`
- `evidence_quote_or_symbol`
- native fields, units, frame, and adapter target

Fields without official documentation, official source, current repository code,
or runnable API introspection evidence stay `unsupported` or `unknown`.

Evidence priority:

1. official documentation
2. official source or message definition
3. current repository code
4. runnable API introspection output
5. papers only as background, not field definitions

## /aircraft/* Naming Norm

The Runtime Observation Layer may be richer than FlightCore, but it is still a
typed observation domain, not a product bus:

| Domain | Meaning | FlightCore use |
|---|---|---|
| `/aircraft/truth/state` | simulator pose, velocity, status, and other ground-truth or estimated runtime state | trace-only |
| `/aircraft/sensors/imu` | IMU measurement | current `/uav/sensors/imu` contract |
| `/aircraft/sensors/gps` | GNSS measurement | current `/uav/sensors/gps` contract |
| `/aircraft/sensors/barometer` | barometric altitude / pressure measurement | future sensor contract candidate |
| `/aircraft/sensors/magnetometer` | magnetic field measurement | future sensor contract candidate |
| `/aircraft/sensors/rangefinder` | distance / range measurement | future sensor contract candidate |
| `/aircraft/sensors/lidar` | lidar point cloud / scan observation | trace-only until a real product contract exists |
| `/aircraft/sensors/camera/*` | camera image or camera metadata | trace / perception path only |
| `/aircraft/environment/wind` | configured or observed wind state where evidence exists | scenario/internal, not control input |
| `/aircraft/actuators/motor_state` | simulator motor/rotor feedback such as thrust, speed, or torque | trace-only |
| `/aircraft/power/battery` | simulator battery telemetry where evidence exists | unsupported until field evidence exists |
| `/aircraft/events/collision` | collision/contact event | scenario/fault and trace only |

Current legacy aliases remain valid while the bridge is migrated:

- `/aircraft/state` maps to the older UDP `state` packet and represents truth /
  runtime observation, not a FlightCore sensor.
- `/aircraft/imu` and `/aircraft/gps` remain the current ROS2 bridge topic names
  for the first working path.

## Product Contract Boundary

`/uav/*` remains the FlightCore contract layer. Today it contains only:

- `/uav/sensors/imu`
- `/uav/sensors/gps`
- `/uav/cmd/flight`
- `/uav/actuator/esc_cmd`
- `/uav/estimator/state`
- `/uav/health/status`

The actuator naming is a known legacy issue. New design language should prefer
`MotorSetpoint` or `ActuatorMotors`: four normalized FlightCore motor setpoints,
not PWM, DShot, RPM, or real thrust. The existing `EscCmd` topic and message are
kept only to avoid breaking the current Simulink/ROS2 path.

## Confirmed Phase-1 Evidence

| Simulator | Confirmed observations | Evidence |
|---|---|---|
| AirSim | IMU, GPS, barometer, magnetometer, distance sensor, lidar, camera image response, collision info, multirotor state, rotor state, wind setter | AirSim sensors/API docs and AirSim source symbols listed in the registry |
| Gazebo Sim Harmonic | IMU, contact, lidar examples with message fields | Gazebo sensors tutorial |
| Isaac Sim | camera/lidar/radar/physics sensor families; IMU frame; contact frame | Isaac Sim sensors, IMU, and contact sensor docs |
| Pegasus | simulator basis on Isaac Sim plus multirotor/PX4/ArduPilot/custom interfaces; `State` API fields confirmed | Pegasus docs/API pages |

## Unsupported Or Unknown In Phase 1

- AirSim battery is `unsupported` here. The official code search found MAVLink
  battery messages/tests, but no simulator-neutral AirSim battery observation API
  suitable for this bridge inventory.
- Pegasus sensor fields beyond the documented `State` object remain
  `unsupported` until Pegasus API/source fields are captured per sensor.
- Gazebo GPS/barometer/magnetometer are `unknown` in this inventory because the
  phase-1 evidence pass used the official Harmonic sensors page examples only.
- Isaac/Pegasus multirotor motor-state fields are not promoted unless a concrete
  API/source symbol is recorded for that simulator.

## Most Valuable Next Packets

1. `sensor_imu`: replaces the legacy state-derived `/aircraft/imu` approximation
   with real simulator IMU samples and preserves the existing `/uav/sensors/imu`
   path.
2. `sensor_gps`: enables real `/aircraft/sensors/gps` input and lets
   `gps_fallback_from_state` stay debug-only.
3. `collision`: gives the adapter a fault/scenario event path without feeding
   collision truth into FlightCore.

`motor_state` is useful for evaluation after the first two sensors are clean,
but it must stay trace-only because it is simulator actuator feedback, not a
FlightCore setpoint.
