# InterfaceContract

> 基线日期：2026-06-24  
> 范围：定义 `FlightCore` 到 `SimAdapter` 以及后续部署适配器的稳定边界。

## 目的

`InterfaceContract` 定义 `FlightCore` 与任意 `RuntimeAdapter` 之间的数据模型。它必须与具体仿真器无关，并同时适用于 InternalSim、AirSim、Pegasus、ROS2、BoardHAL、HIL 和 SIL。

后端专用 API 禁止出现在 `FlightCore` 侧。运行时名称、连接设置、仿真器步进、载具绑定、场景配置、旋翼 API 调用、中间件话题和板级驱动细节都必须留在 adapter 内部。

```text
RuntimeAdapter -> SensorInput / CommandInput / EstimatorInit -> FlightCore
FlightCore -> ActuatorOutput / Telemetry -> RuntimeAdapter
```

## 坐标与时间基线

- 平动状态默认使用本地 NED 坐标系，除非字段显式说明其他坐标系。
- 机体系向量使用 FRD。
- 姿态四元数使用 `[w x y z]` 顺序。
- 传感器采样时间以秒为单位。
- 传感器总线必须携带 `Valid` 和 `Timestamp`。
- 新样本检测必须使用 `Timestamp` 或后续新增的 `Sequence` 字段，不得依赖测量值变化。

## SensorInput

`SensorInput` 将运行时测量输入 `FlightCore` 的估计器和健康监控逻辑。

当前 Simulink 总线：

- `IMU_BUS`
  - `Accel`: single[3], body FRD, m/s^2
  - `Gyro`: single[3], body FRD, rad/s
  - `Valid`: boolean
  - `Timestamp`: double, seconds
- `GPS_BUS`
  - `Lat`: single, deg
  - `Lon`: single, deg
  - `Alt`: single, m
  - `Velocity`: single[3], local NED, m/s
  - `Valid`: boolean
  - `Timestamp`: double, seconds

后续气压计、磁力计、空速计、测距仪和时间同步等传感器必须遵循同样的有效性与时间戳规则。

## CommandInput

`CommandInput` 将操作员、任务或自主逻辑的指令输入命令仲裁层。

当前 Simulink 总线：

- `FlightCmdBus`
  - `Position_NED_SP`: single[3], m
  - `Velocity_NED_SP`: single[3], m/s
  - `Yaw_SP`: single, rad
  - `Mode`: uint8
  - `Valid`: boolean

`CommandInput` 不得携带仿真器连接数据、场景初始条件、估计器状态或控制器内部状态。

## ActuatorOutput

`ActuatorOutput` 将 `FlightCore` 输出传递给被控对象、仿真器、中间件桥或板级驱动。

当前 Simulink 总线：

- `EscCmdBus`
  - `MotorCmd`: single[4], normalized 0..1

Adapter 负责执行器传输、运行时边界限幅策略、后端电机顺序映射和超时处理。

## EstimatorInit

`EstimatorInit` 向 `FlightCore` 和 adapter 复位逻辑提供初始化数据。

必需语义：

- 本地 NED 初始位置
- 本地 NED 初始速度
- 初始姿态四元数 `[w x y z]`
- FRD 机体系初始角速率
- 陀螺零偏
- 加速度计零偏
- 协方差对角线或完整协方差
- 本地原点 LLA
- 有效性标志
- 来源标识

`SimulationDict` 是当前 Simulink 中该类数据的来源。它是场景配置源，不是运行时数据总线。

## Telemetry

`Telemetry` 将可观测性数据从 `FlightCore` 输出到运行时。

最小语义：

- 状态估计
- 估计器状态
- 控制器状态
- 执行器饱和状态
- 健康与故障安全状态
- 当 adapter 能提供真值时，可包含运行时真值对比

`Telemetry` 不是 `FlightCore` 的控制输入。

## ROS2 Gateway 最小契约

ROS2 是 Gateway 传输映射，不是 FlightCore 内部事实源。当前 MBD 实现将稳定的 FlightCore Bus 契约映射到 `FC_SimulinkProject/3_Integration/ROS2/flightcore_msgs` 下的自定义 ROS2 消息。

`FC_SimulinkProject/3_Integration/ROS2/flightcore_msgs/msg/` 是 MATLAB/Simulink 侧 ROS2 消息定义源。Topic、QoS 和 Bus 字段映射由 `FlightCore_ROS2_loop.slx` 的 ROS2 块及边界适配子系统定义。

最小 ROS2 topic：

| Topic | Message | Direction | FlightCore 映射 |
|---|---|---|---|
| `/uav/sensors/imu` | `flightcore_msgs/Imu` | subscribe | `IMU_BUS` |
| `/uav/sensors/gps` | `flightcore_msgs/Gps` | subscribe | `GPS_BUS` |
| `/uav/cmd/flight` | `flightcore_msgs/FlightCmd` | subscribe | `FlightCmdBus` |
| `/uav/actuator/esc_cmd` | `flightcore_msgs/EscCmd` | publish | `EscCmdBus` |
| `/uav/estimator/state` | `flightcore_msgs/StateEst` | publish | `StateEstBus` |
| `/uav/health/status` | `flightcore_msgs/SystemHealth` | publish | ROS2 Gateway health |

消息边界规则：

- 传感器和命令消息可以携带 `stamp`、`timestamp_sec`、`sequence`、`source_id`，用于 Gateway 同步和诊断。
- 只有 `FlightCore_ROS2_loop.slx` 边界适配子系统显式映射的字段进入当前 Simulink Bus 对象。
- `RuntimeTruth`、trace、rosbag2 元数据、仿真器身份、DDS 细节和可视化专用字段不得进入 `FlightCore`。
- `std_msgs/Bool` 和 `std_msgs/Float32` 不是有效的 FlightCore 接口契约；只能作为 FlightCore 边界外的临时诊断消息。

## 变更规则

接口契约发生变化时，必须先在 `FC_SimulinkProject/4_Test/` 下补充测试，再修改模型、adapter、bridge 或部署代码。
