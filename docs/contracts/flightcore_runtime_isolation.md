# FlightCore Runtime Isolation Contract

> Baseline date: 2026-06-23  
> Scope: UAV single-aircraft flight-control architecture  
> Authority: this document defines the boundary between flight-control product logic and all simulation/deployment runtimes.

## Purpose

The project architecture is centered on:

```text
FlightCore + InterfaceContract + RuntimeAdapter
```

The purpose of this contract is to keep the flight-control product independent from any simulator, middleware, or board support package. Simulators and deployment targets may change; the flight-control interface must remain stable.

## FlightCore Boundary

FlightCore contains only product flight-control responsibilities:

- state estimation selection and execution
- position, velocity, attitude, and rate control
- mixer and actuator allocation logic
- command validation and command arbitration
- later navigation, mission management, mode management, and failsafe logic
- health status and flight-control telemetry generation

FlightCore consumes and produces only the project interface buses. It must not own scene setup, physics-world configuration, sensor rendering, transport protocols, external process lifecycle, or board driver details.

FlightCore may use subsystem dictionaries for product parameters such as controller gains, estimator parameters, mixer geometry, and vehicle physical parameters. It must not read scenario-only settings as product configuration.

## Interface Contract

InterfaceContract is the stable boundary between FlightCore and RuntimeAdapter. The first implementation may map these contracts to Simulink Bus objects; later implementations may map the same semantics to C++ structs, ROS messages, or shared-memory packets.

### SensorInput

SensorInput carries measurements into the estimator and health-monitoring path.

- `IMU_BUS`: acceleration, angular rate, validity, timestamp, and optional sequence.
- `GPS_BUS`: position, velocity, validity, timestamp, and optional sequence.
- Future buses: barometer, magnetometer, airspeed, rangefinder, and time sync.

All sensor buses must carry `Valid` and `Timestamp`. New-sample detection must use timestamp or sequence, not measurement-value changes.

### CommandInput

CommandInput carries operator, mission, or autonomy commands into command arbitration.

- `FlightCmdBus`: position setpoint, velocity setpoint, yaw setpoint, mode, and validity.
- Future buses: RC input, mission item, offboard setpoint, arm/disarm request.

CommandInput must not carry scenario initial conditions, environment connection data, or controller internal state.

### ActuatorOutput

ActuatorOutput carries FlightCore commands to the runtime environment.

- `EscCmdBus`: normalized motor commands and validity.
- Future buses: servo commands, actuator status requests, and actuator enable state.

ActuatorOutput is the only path from FlightCore to a plant, simulator, middleware bridge, or board driver.

### EstimatorInit

EstimatorInit, also called InitContext in early notes, initializes estimator and truth-aligned runtime state.

- initial position in local NED
- initial velocity in local NED
- initial attitude quaternion
- initial angular rate
- gyro and accelerometer bias
- covariance diagonal or full covariance
- local origin LLA
- validity flag
- source identifier

SimulationDict may provide EstimatorInit for internal simulation. Deployment code may provide it from boot-time alignment, stored calibration, or external navigation initialization. The estimator consumes the same contract in all cases.

### Telemetry

Telemetry carries observability data out of FlightCore.

- state estimate
- estimator status
- controller status
- actuator saturation status
- health and failsafe status
- debug truth comparison when a runtime can provide truth data

Telemetry is not a control input. RuntimeAdapter may log it or forward it externally.

## Runtime Adapters

RuntimeAdapter is responsible for converting each runtime backend to and from InterfaceContract. Each adapter owns backend-specific APIs, coordinate conversions, time synchronization, process lifecycle, connection health, and fail-safe behavior at the boundary.

### InternalSimAdapter

InternalSimAdapter maps the current Simulink closed loop into the interface contract.

- reads DynamicModel and sensor models as the internal environment
- provides SensorInput and EstimatorInit to FlightCore
- consumes ActuatorOutput from FlightCore
- may provide truth data for Telemetry comparison

This adapter is the current regression backend and should remain the fastest path for interface and controller checks.

### PegasusAdapter (Future)

PegasusAdapter will map Isaac Sim / Pegasus vehicle state, sensors, and vehicle actuation to InterfaceContract. It is a future candidate — no implementation work has started. AirSim is the current external simulator adapter.

### AirSimAdapter (Current)

AirSimAdapter is the current external simulator adapter.

- converts AirSim IMU/GPS data to SensorInput
- converts ActuatorOutput to AirSim motor commands or visual pose sync depending on phase
- owns AirSim connection setup, pause/step behavior, coordinate conversion, and rotor order mapping

AirSim API names and settings must not appear in FlightCore models or product dictionaries.

### ROS2Adapter

ROS2Adapter maps InterfaceContract to ROS2 topics, services, and time.

- publishes Telemetry and ActuatorOutput where required
- subscribes to SensorInput and CommandInput providers
- owns QoS, topic naming, message versioning, and time synchronization

ROS2 is middleware, not the FlightCore data model.

### ROS2 Runtime Layer

The preferred long-term runtime isolation shape is to treat ROS2 as the runtime integration layer between FlightCore and the outside world, rather than as a visualization-only add-on.

In this shape, simulators, hardware, perception, video, monitoring, and logging all meet through ROS2 topics:

```text
Simulator / Sensors / Perception / Board I/O
  -> ROS2 Runtime Layer
  -> ROS2Adapter
  -> InterfaceContract
  -> FlightCore
  -> InterfaceContract
  -> ROS2Adapter
  -> ROS2 Runtime Layer
  -> PlotJuggler / RViz / rosbag2 / frontend / actuator bridge
```

ROS2 Runtime Layer responsibilities:

- normalize simulator and hardware data into stable ROS2 topics
- publish sensor, truth, command, actuator, trace, health, and video streams
- own QoS, topic namespace, message versioning, rosbag2 recording, and clock/time synchronization
- feed PlotJuggler and other monitoring tools without adding visualization logic to FlightCore
- allow AirSim, Isaac/Pegasus, Gazebo, HIL, SIL, and board deployments to share the same runtime contract

FlightCore responsibilities do not change:

- it must not depend on simulator APIs, ROS2 node lifecycle, PlotJuggler, RViz, rosbag2, DDS vendor behavior, or video transport details
- it continues to consume and produce InterfaceContract data
- it may be connected to ROS2 through ROS2Adapter, but ROS2 message definitions are transport mappings, not the product data model

Initial topic sketch:

```text
/uav/sensor/imu
/uav/sensor/gps
/uav/truth/state
/uav/cmd/flight
/uav/output/esc_cmd
/uav/trace/experiment
/uav/health/status
/uav/video/front/image
/uav/video/front/camera_info
```

Current development-phase implementation:

- FlightCore_ROS2_loop.slx runs in Windows MATLAB using Simulink ROS Toolbox for the /uav/* six-topic contract.
- The WSL Python flightcore_runtime_adapter handles /aircraft/*→/uav/* semantic conversion and UDP actuator feedback.
- PlotJuggler + rosbag2 on WSL are the primary trace/logging tools.
- The DDS cross-boundary exception (see subsection below) applies during this phase.

### DDS Cross-Boundary Exception (Development Phase Only)

**Context:** The architecture rule ("Windows↔WSL2 is UDP-only; no DDS/ROS2 cross-boundary") was designed for the production deployment where FlightCore runs as generated code inside the WSL/Linux environment. During the **development phase**, FlightCore runs as a Simulink model in Windows MATLAB and must consume /uav/* topics over DDS.

**Exception:** The six /uav/* `flightcore_msgs` topics (`/uav/sensors/imu`, `/uav/sensors/gps`, `/uav/cmd/flight`, `/uav/actuator/esc_cmd`, `/uav/estimator/state`, `/uav/health/status`) are permitted to cross the Windows↔WSL2 boundary via DDS best-effort QoS during the Simulink-based development phase.

**Boundaries:**
- Only these six topics; no `/aircraft/*` DDS cross-boundary.
- Only MATLAB ROS Toolbox subscribers/publishers on the Windows side. No FlightCore ROS2 nodes on Windows outside MATLAB.
- The UDP-only rule remains in full force for the simulator endpoint path (Windows AirSim → WSL2 `aircraft_udp_bridge`).

**Sunset condition:** This exception is automatically revoked when FlightCore's generated code moves into WSL/Linux. At that point:
1. /uav/* topics no longer need to cross Windows↔WSL2.
2. The UDP-only rule becomes universal — all Windows↔WSL2 traffic goes through the UDP port pair (56000/56001).
3. This exception section must be deleted from this document.

**Transition triggers (all three must be met before the exception sunsets):**
1. External closed-loop episode (AirSim→WSL→FlightCore) has been demonstrated and recorded as an episode.
2. FlightCore control/estimation logic enters low-frequency modification phase (six-topic contract already frozen — partially met).
3. M6 code-generation milestone officially starts.

### BoardHALAdapter

BoardHALAdapter maps InterfaceContract to embedded board drivers and generated code integration.

- reads physical IMU, GPS, and future sensors into SensorInput
- writes ActuatorOutput to PWM, DShot, CAN, or other board outputs
- owns board timing, driver error handling, and hardware status

BoardHALAdapter is the deployment peer of simulator adapters. It must use the same contract, so code generation does not inherit simulator assumptions.

### HILAdapter

HILAdapter combines hardware I/O and external plant execution while preserving InterfaceContract.

- provides real-time sensor and actuator transport
- owns latency measurement and timeout handling
- logs timing and dropped-frame diagnostics

### SILAdapter

SILAdapter executes generated or hand-written code against recorded or simulated InterfaceContract data.

- compares generated-code outputs with model outputs
- supports regression replay
- isolates compiler and numeric differences from simulator integration work

## Implementation Order

1. The current `3_Integration/UAV_FC_loop.slx` is the internal closed-loop regression backend (zero maintenance, kept for regression diagnostics when external closed-loop fails).
2. Use `3_Integration/FlightCore/FlightCore.slx` as the explicit FlightCore top level.
3. Treat the current `FlightCore.slx` as the product core boundary: `IMU_BUS + GPS_BUS -> CommandGenerator + EKF + UAV_FlightControl -> EscCmdBus + StateEstBus`.
4. Add InterfaceContract tests before moving buses or model references.
5. AirSim is the current external simulator adapter. The AirSim endpoint is the only place where AirSim API names may appear.
6. ROS2Adapter implementation: flightcore_runtime_adapter (WSL Python package) handles /aircraft/*→/uav/* conversion. FlightCore_ROS2_loop.slx uses Simulink ROS Toolbox for the /uav/* six-topic contract.
7. PegasusAdapter, BoardHALAdapter, HILAdapter, and SILAdapter are design-level stubs only until their respective milestones start.

## ScenarioConfig

ScenarioConfig configures a run, not the runtime data path.

ScenarioConfig is not a runtime data bus.

Allowed ScenarioConfig content:

- initial position, velocity, attitude, and angular rate
- local origin and initial estimator covariance
- wind, disturbance, and environment settings
- runtime backend selection
- simulation stop time and logging settings

Forbidden ScenarioConfig content:

- controller gains
- estimator algorithm internals
- mixer geometry
- product safety thresholds
- transport protocol state
- actuator commands
- live sensor measurements

SimulationDict is the current Simulink implementation of ScenarioConfig and EstimatorInit source data. It must not replace InterfaceContract.

## Acceptance Rules

- FlightCore models consume only InterfaceContract inputs and product dictionaries.
- RuntimeAdapter models or scripts are the only place where backend-specific API names may appear.
- A new simulator backend must first map SensorInput, CommandInput, ActuatorOutput, EstimatorInit, and Telemetry.
- Any backend-specific field requested by FlightCore is treated as an architecture violation until converted into a general contract field.
- Contract changes require tests under `FC_SimulinkProject/4_Test/` before model or adapter edits.
