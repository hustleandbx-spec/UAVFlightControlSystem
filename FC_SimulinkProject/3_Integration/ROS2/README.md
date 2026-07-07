# FlightCore ROS2 Interface Contract

This directory contains the ROS2 topic and message mapping for the FlightCore Simulink
Bus interface.

Recommended mainline runtime path:

```text
ROS2 sensors / command providers
  -> /uav/sensors/imu + /uav/sensors/gps + /uav/cmd/flight
  -> Simulink ROS Toolbox Subscribe
  -> ROS2 message to FlightCore Bus adapter
  -> FlightCore
  -> FlightCore Bus to ROS2 message adapter
  -> Simulink ROS Toolbox Publish
  -> /uav/actuator/esc_cmd + /uav/estimator/state + /uav/health/status
  -> PlotJuggler / rosbag2 / monitors
```

This keeps ROS2 as the communication middleware while preserving the FlightCore
Bus boundary. FlightCore must not directly depend on ROS2 message classes,
DDS QoS policy, PlotJuggler, or gateway/runtime adapter implementation details.

**Note:** During the development phase, this path crosses Windows↔WSL2 via DDS
(see DDS Cross-Boundary Exception in `docs/contracts/flightcore_runtime_isolation.md`).
This exception sunsets when FlightCore generated code moves into WSL/Linux.

Minimum topics:

| Topic | Message | Direction | Simulink bus |
|---|---|---|---|
| `/uav/sensors/imu` | `flightcore_msgs/Imu` | subscribe | `IMU_BUS` |
| `/uav/sensors/gps` | `flightcore_msgs/Gps` | subscribe | `GPS_BUS` |
| `/uav/cmd/flight` | `flightcore_msgs/FlightCmd` | subscribe | `FlightCmdBus` |
| `/uav/actuator/esc_cmd` | `flightcore_msgs/EscCmd` | publish | `EscCmdBus` |
| `/uav/estimator/state` | `flightcore_msgs/StateEst` | publish | `StateEstBus` |
| `/uav/health/status` | `flightcore_msgs/SystemHealth` | publish | ROS2 boundary health |

`topics.yaml` is the source of truth for topic names, message names, QoS intent,
and Bus field mappings. The `.msg` files are the ROS2 build inputs. ROS2-specific
metadata such as `stamp`, `source_id`, and `sequence` stays at the ROS2 boundary
unless the Bus contract explicitly maps it; only mapped fields enter current
FlightCore Bus objects.

Build sketch in a ROS2 workspace:

```bash
mkdir -p ros2_ws/src
cp -r flightcore_msgs ros2_ws/src/
cd ros2_ws
colcon build --packages-select flightcore_msgs
```

WSL2/Jazzy quick build:

```bash
# Build on WSL native workspace (copy flightcore_msgs from Windows first if changed)
cd ~/uavsingle_ros2_ws
source /opt/ros/jazzy/setup.bash
colcon build --packages-select flightcore_msgs --merge-install --symlink-install
source install/setup.bash
```

After building:

```bash
source ~/uavsingle_ros2_ws/install/setup.bash
ros2 interface show flightcore_msgs/msg/Imu
ros2 topic pub /uav/sensors/imu flightcore_msgs/msg/Imu "{valid: true, timestamp_sec: 0.0, accel_mps2: [0.0, 0.0, -9.80665], gyro_radps: [0.0, 0.0, 0.0]}"
```

Windows MATLAB ROS Toolbox communication probe:

- WSL2 to MATLAB has been verified with `demo_nodes_cpp talker` and MATLAB
  `ros2subscriber`.
- MATLAB to WSL2 requires explicit best-effort QoS for the current WSL2/Jazzy
  test path.

PlotJuggler:

```bash
source ~/uavsingle_ros2_ws/install/setup.bash
plotjuggler -l ~/uavsingle_ros2_ws/config/plotjuggler_flightcore_topics.xml
```

The PlotJuggler layout is a WSL runtime file:

```text
~/uavsingle_ros2_ws/config/plotjuggler_flightcore_topics.xml
```

Subscribe to:

- `/uav/actuator/esc_cmd`
- `/uav/estimator/state`
- `/uav/health/status`

MATLAB/Simulink note:

`FlightCore_ROS2_loop.slx` can only switch its ROS2 blocks from `std_msgs/*`
to `flightcore_msgs/*` after MATLAB ROS Toolbox has registered the custom
messages on Windows. The model must keep a Simulink-internal adapter boundary:

```text
ROS2 message
  -> ROS2ToFlightCoreBus
  -> IMU_BUS / GPS_BUS / FlightCmdBus
  -> FlightCore
  -> EscCmdBus / StateEstBus
  -> FlightCoreBusToROS2
  -> ROS2 message
```

Do not hard-edit the `.slx` model to claim `flightcore_msgs/*` support until
MATLAB can resolve the custom messages and update compile succeeds.

Custom message registration:

```powershell
.\scripts\configure_matlab_ros2_custom_messages_isolated.ps1
```

The isolated script uses `D:\Miniconda\envs\airsim\python.exe` because MATLAB
ROS Toolbox R2025b rejects Python 3.12/3.14 for ROS2 custom message generation.
It also uses a project-local temporary MATLAB home under `.cache/` to avoid the
known `C:\Users\89447\source` conflict reported by `ros2genmsg`.
