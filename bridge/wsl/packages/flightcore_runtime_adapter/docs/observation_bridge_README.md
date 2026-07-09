# Simulator ROS2 UDP Bridge

This bridge keeps the Windows/WSL2 boundary UDP-only and keeps ROS2/DDS inside
WSL2. The runtime layering is:

```text
Simulator endpoint
  -> UDP state packet
  -> aircraft_udp_bridge
  -> /aircraft/*
  -> flightcore_runtime_adapter
  -> /uav/*
  -> FlightCore_ROS2_loop
  -> /uav/actuator/esc_cmd
  -> flightcore_runtime_adapter
  -> UDP actuator packet
  -> simulator endpoint
```

Layer names:

- `/aircraft/*` = Aircraft Runtime Observation Layer.
- `/uav/*` = FlightCore Contract Layer.

The simulator endpoint is the only place that may use a simulator-native API.
The runtime adapter only consumes generic `/aircraft/*` ROS2 messages and
publishes `flightcore_msgs` contracts for FlightCore.

## Boundary Rules

- Windows <-> WSL2 uses UDP JSON datagrams only.
- Do not enable ROS2/DDS discovery across Windows/WSL2.
- Do not run FlightCore ROS2 nodes on Windows for this path.
- Do not place simulator-native API calls in WSL2 ROS2 packages.
- FlightCore must subscribe only to `/uav/*` `flightcore_msgs` topics.
- `/aircraft/state` is the current legacy alias for `/aircraft/truth/state`.
  It is truth/observation data for monitoring, logging, and evaluation. It is
  not a FlightCore control-loop sensor input.
- `/aircraft/environment/*`, `/aircraft/events/*`, `/aircraft/actuators/*`,
  `/aircraft/truth/*`, and `/aircraft/sensors/camera/*` must not be bridged
  directly into `/uav/*`.

## Ports

| Flow | Default |
|---|---|
| Simulator endpoint -> WSL2 state | UDP `56000` |
| WSL2 -> simulator endpoint command/actuator | UDP `56001` |

`aircraft_udp_bridge` can learn the simulator endpoint host from incoming state
packets for the legacy debug control path. `flightcore_runtime_adapter` sends
actuator packets directly, so set `actuator_target_host` when the endpoint is
not reachable at the default host you choose for your environment.

## Protocol

The protocol is fixed JSON per UDP datagram. See:

- `protocol.md`
- `schemas/state.schema.json`
- `schemas/sensor_imu.schema.json`
- `schemas/sensor_gps.schema.json`
- `schemas/sensor_barometer.schema.json`
- `schemas/sensor_magnetometer.schema.json`
- `schemas/sensor_rangefinder.schema.json`
- `schemas/sensor_lidar.schema.json`
- `schemas/environment.schema.json`
- `schemas/collision.schema.json`
- `schemas/motor_state.schema.json`
- `schemas/control.schema.json`
- `schemas/actuator.schema.json`

State packets use local NED position/velocity and aircraft FRD body rates. New
observation packets use `packet_type` values such as `sensor_imu`, `sensor_gps`,
`environment`, `collision`, and `motor_state`.

The closed-loop FlightCore return path uses actuator packets:

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

The old `control` packet remains available for rate/attitude debug and mock
testing. It is not the FlightCore closed-loop main path.

`motor_cmd` is a normalized FlightCore motor setpoint. New design documents
should prefer `MotorSetpoint` or `ActuatorMotors`; the existing `EscCmd` name is
kept only for compatibility with the current Simulink/ROS2 mapping.

## WSL2 ROS2 Jazzy Build

Maintain and build the ROS2 workspace on the WSL-native filesystem, not from a
Windows-mounted path. The current WSL workspace is:

```bash
~/uavsingle_ros2_ws
```

Place these packages under `~/uavsingle_ros2_ws/src/`:

- `flightcore_msgs`
- `aircraft_udp_bridge`
- `flightcore_runtime_adapter`

Use a single merge-install prefix so `install/setup.bash` exposes both Python
packages and interface packages consistently:

```bash
cd ~/uavsingle_ros2_ws
source /opt/ros/jazzy/setup.bash
colcon build --merge-install --symlink-install
source install/setup.bash
```

Expected post-build checks:

```bash
ros2 pkg prefix flightcore_msgs
ros2 pkg executables aircraft_udp_bridge
ros2 pkg executables flightcore_runtime_adapter
ros2 interface show flightcore_msgs/msg/EscCmd
```

Run the simulator observation bridge:

```bash
ros2 run aircraft_udp_bridge aircraft_udp_bridge --ros-args \
  -p state_bind_host:=0.0.0.0 \
  -p state_bind_port:=56000
```

Run the FlightCore runtime adapter:

```bash
ros2 run flightcore_runtime_adapter flightcore_runtime_adapter --ros-args \
  -p actuator_target_host:=<SIMULATOR_ENDPOINT_IP> \
  -p actuator_target_port:=56001
```

Useful adapter parameters:

```bash
ros2 run flightcore_runtime_adapter flightcore_runtime_adapter --ros-args \
  -p source_id:=1 \
  -p publish_default_flight_cmd:=true \
  -p flight_cmd_mode:=1 \
  -p flight_cmd_position_ned_m:="[0.0, 0.0, -2.0]" \
  -p gps_fallback_from_state:=false
```

`gps_fallback_from_state:=true` synthesizes `/uav/sensors/gps` from local NED
state plus `local_origin_*` parameters. This is a temporary debug path only; the
preferred path is a real `/aircraft/gps` `sensor_msgs/msg/NavSatFix` source.

Inspect ROS2 output inside WSL2:

```bash
ros2 topic echo /aircraft/state
ros2 topic echo /aircraft/imu
ros2 topic echo /uav/sensors/imu
ros2 topic echo /uav/sensors/gps
ros2 topic echo /uav/cmd/flight
```

## Windows Simulator Endpoint

Run the endpoint on Windows and point it at the WSL2 IP:

```powershell
wsl hostname -I
cd D:\Project\UAVSingleFlightControl\bridge\airsim_ros2_udp_bridge
python .\windows\airsim_udp_endpoint.py --bridge-host <WSL2_IP> --rate-hz 50
```

Common options:

```powershell
python .\windows\airsim_udp_endpoint.py `
  --bridge-host <WSL2_IP> `
  --state-port 56000 `
  --control-port 56001 `
  --vehicle-name Drone1 `
  --enable-api-control `
  --arm `
  --motor-order 0,1,2,3
```

`--motor-order` maps endpoint motor API argument order to FlightCore
`motor_cmd` indexes. The default `0,1,2,3` is only a neutral pass-through; set it
for the simulator vehicle geometry you are using.

If the endpoint client exposes `moveByMotorPWMsAsync`, actuator packets are sent
through that API. If the API is unavailable, the endpoint logs an unsupported
fallback warning and keeps running. Mock mode logs received actuator packets:

```powershell
python .\windows\airsim_udp_endpoint.py --bridge-host <WSL2_IP> --mock --duration-sec 5
```

## Topics

Aircraft Runtime Observation Layer:

| Topic | Message | Direction |
|---|---|---|
| `/aircraft/truth/state` | `nav_msgs/msg/Odometry` | target name for truth/runtime state |
| `/aircraft/state` | `nav_msgs/msg/Odometry` | current legacy alias published by `aircraft_udp_bridge` |
| `/aircraft/sensors/imu` | `sensor_msgs/msg/Imu` | target name for simulator IMU samples |
| `/aircraft/imu` | `sensor_msgs/msg/Imu` | current legacy alias published by `aircraft_udp_bridge` |
| `/aircraft/sensors/gps` | `sensor_msgs/msg/NavSatFix` | target name for simulator GNSS samples |
| `/aircraft/gps` | `sensor_msgs/msg/NavSatFix` | current optional bridge alias |
| `/aircraft/sensors/barometer` | TBD | observation registry only in phase 1 |
| `/aircraft/sensors/magnetometer` | TBD | observation registry only in phase 1 |
| `/aircraft/sensors/rangefinder` | TBD | observation registry only in phase 1 |
| `/aircraft/sensors/lidar` | TBD | observation registry only in phase 1 |
| `/aircraft/sensors/camera/*` | image/camera metadata TBD | trace/perception path only |
| `/aircraft/environment/wind` | TBD | scenario/internal state only |
| `/aircraft/actuators/motor_state` | TBD | trace-only simulator feedback |
| `/aircraft/power/battery` | unsupported | no phase-1 field evidence |
| `/aircraft/events/collision` | TBD | scenario/fault and trace only |
| `/aircraft/control` | `std_msgs/msg/String` JSON control packet | legacy debug input |
| `/aircraft/cmd_vel` | `geometry_msgs/msg/Twist` | legacy debug input |

FlightCore Contract Layer:

| Topic | Message | Direction |
|---|---|---|
| `/uav/sensors/imu` | `flightcore_msgs/msg/Imu` | adapter publishes, FlightCore subscribes |
| `/uav/sensors/gps` | `flightcore_msgs/msg/Gps` | adapter publishes, FlightCore subscribes |
| `/uav/cmd/flight` | `flightcore_msgs/msg/FlightCmd` | adapter publishes, FlightCore subscribes |
| `/uav/actuator/esc_cmd` | `flightcore_msgs/msg/EscCmd` | FlightCore publishes, adapter subscribes |

## Legacy Debug Control

The old rate/attitude path is still useful for smoke tests:

```bash
ros2 topic pub --once /aircraft/control std_msgs/msg/String \
  "{data: '{\"protocol_version\":1,\"packet_type\":\"control\",\"timestamp\":0.0,\"sequence\":1,\"mode\":\"rate\",\"throttle\":0.45,\"roll\":0.0,\"pitch\":0.0,\"yaw\":0.0}'}"
```

Or use `/aircraft/cmd_vel`:

```bash
ros2 topic pub --once /aircraft/cmd_vel geometry_msgs/msg/Twist \
  "{linear: {z: 0.45}, angular: {x: 0.0, y: 0.0, z: 0.0}}"
```

`/aircraft/cmd_vel` maps `linear.z -> throttle`,
`angular.x/y/z -> roll/pitch/yaw`, with `cmd_vel_mode` defaulting to `rate`.
