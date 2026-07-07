# FlightCore 可解释实验基座开发计划

> 日期：2026-06-29  
> 主线：`AirSim -> RuntimeBridge -> SimAdapter -> FlightCore -> Shadow/Trace`  
> 目标：先建立可解释、可记录、可比较的飞控实验基座，再讨论仿真器、控制器或智能算法优劣。

---

## 0. 核心判断

当前阶段不优先纠结仿真器，也不优先优化控制器。

原因是：

```text
仿真器本身需要被基座评估
控制器本身需要被基座评估
估计器本身需要被基座评估
AirSim / Isaac / PX4 / RL / 世界模型都应回到同一套实验事实中证明价值
```

因此，当前优先级最高的不是“换一个更高级模块”，而是让现有链路从黑箱变成可解释系统。

当前真实主线是：

```text
AirSim
  -> RuntimeBridge
  -> SensorFrame / CommandFrame
  -> SimAdapter
  -> FlightCore
  -> ShadowOutput
  -> ShadowFrame / CSV / actuator write
```

开发目标是让这条链路能够回答：

```text
AirSim 真值是什么？
传感器输入是什么？
FlightCore 估计了什么？
控制器目标是什么？
控制器为什么输出这些电机量？
估计误差如何变化？
是否饱和？
是否延迟？
是否失效？
失败原因是什么？
```

---

## 1. 当前事实

已有基础：

```text
FlightCore_SimAdapter_loop 可 update compile
SimAdapter 已有 NetworkSensorInput / NetworkCommandInput / ShadowOutput
RuntimeBridge 已有 SensorFrame / CommandFrame / ShadowFrame
SensorFrame 已携带 AirSim truth_position_ned / truth_velocity_ned
ShadowFrame 已携带 motor_cmd / estimated position / estimated velocity
SignalLogging 已开启
```

核心缺口：

```text
truth 已进入 RuntimeBridge，但没有进入模型实验记录
FlightCore 只输出 EscCmdBus / StateEstBus，没有 Telemetry
控制器内部误差、期望量、饱和状态不可见
EKF 内部更新状态、创新、协方差、传感器门控不可见
logsout 开启但没有关键 logged signal
ShadowFrame 太瘦，不能解释系统行为
没有统一 ExperimentTrace / episode 数据
没有自动指标和失败判据
```

---

## 2. 总体架构目标

新增三层可观测性：

```text
RuntimeTruthBus
  AirSim / RuntimeAdapter 提供的运行时真值，只用于监控和评估，不进入 FlightCore 控制闭环。

FlightCore TelemetryBus
  FlightCore 对外暴露的状态、控制、估计、健康、故障安全与调试信息。

ExperimentTrace / ShadowFrameV2
  一次实验每个采样点的完整记录帧，用于 CSV、MAT、Python 分析、指标计算和后续 world model 数据集。
```

推荐数据流：

```text
RuntimeBridge SensorFrame
  -> SimAdapter
    -> IMU_BUS / GPS_BUS / FlightCmdBus -> FlightCore
    -> RuntimeTruthBus ----------------\

FlightCore
  -> EscCmdBus
  -> StateEstBus
  -> TelemetryBus ---------------------\

SimAdapter TraceMux
  -> ExperimentTraceBus / ShadowFrameV2
  -> UDP CSV logger
  -> Simulink logsout
```

硬原则：

```text
可观测性不能反向影响控制闭环
RuntimeTruthBus 不能进入 FlightCore 作为控制输入
Telemetry 不是控制输入
ShadowFrameV2 必须版本化，兼容旧 ShadowFrame
先记录，再评价；先解释，再优化
```

---

## 3. 阶段 P0：冻结当前链路认知

目标：确认当前主线和现状，避免继续误把旧 loop 当主对象。

产物：

```text
inspection_reports/
  flightcore_simadapter_observability_*.md
  flightcore_simadapter_compiled_*.md
```

已完成：

```text
FlightCore_SimAdapter_loop update compile 通过
SimAdapter / FlightCore / EKF / UAV_FlightControl 已被只读检查
ShadowOutput 当前 payload 已确认
RuntimeBridge protocol 已确认
```

保留脚本：

```text
FC_SimulinkProject/3_Integration/inspect_flightcore_simadapter_observability.m
FC_SimulinkProject/3_Integration/inspect_flightcore_simadapter_compiled.m
```

验收标准：

```text
能够一键生成当前模型结构、观测出口、日志设置、Shadow payload、UDP协议报告
后续每次大改前后都能重跑报告对比
```

---

## 4. 阶段 P1：RuntimeTruthBus

目标：把 AirSim 真值从 RuntimeBridge/UDP 帧中显式接入实验记录层。

修改范围：

```text
1_Data_Dictionaries/BusConfig/config_RuntimeTruthBus.m       新增
1_Data_Dictionaries/create_GlobalTypes.m                     自动纳入新 Bus
3_Integration/SimAdapter/SimAdapter.slx                      修改
3_Integration/SimAdapter/SimAdapterSubsystem.slx             同步
3_Integration/RuntimeBridge/protocol/sensor_frame.py         保持 v1，确认字段语义
4_Test/test_runtime_truth_contract.m                         新增
```

`RuntimeTruthBus` 最小字段：

```text
Position_NED       single[3]  m
Velocity_NED       single[3]  m/s
Valid              boolean
Timestamp          double     s
Sequence           uint32
SourceId           uint16
```

SimAdapter 修改：

```text
NetworkSensorInput 中 Byte Unpack 已经解出 truth_position_ned / truth_velocity_ned
将它们接入 RuntimeTruthBusCreator
RuntimeTruthBus 不输入 FlightCore
RuntimeTruthBus 只进入 TraceMux / ShadowOutputV2
```

验收标准：

```text
SimAdapter 顶层能看到 RuntimeTruthBus 或 TraceMux 内部能消费 RuntimeTruthBus
RuntimeTruthBus 字段与 SensorFrame truth 字段一致
FlightCore 中不得出现 RuntimeTruthBus 输入
```

---

## 5. 阶段 P2：FlightCore TelemetryBus

目标：FlightCore 不再只输出控制结果，还要输出解释自身行为的遥测。

修改范围：

```text
1_Data_Dictionaries/BusConfig/config_TelemetryBus.m           新增
1_Data_Dictionaries/BusConfig/config_ControlDebugBus.m        新增
1_Data_Dictionaries/BusConfig/config_EstimatorDebugBus.m      新增
3_Integration/FlightCore/FlightCore.slx                       增加 TelemetryBus outport
2_Model/control/UAV_FlightControl.slx                         增加 ControlDebugBus outport
2_Model/state_estimation/EKF/EKF.slx                          增加 EstimatorDebugBus outport
docs/architecture/interface_contract.md                       更新 Telemetry 语义
4_Test/test_flightcore_telemetry_contract.m                   新增
```

`ControlDebugBus` 最小字段：

```text
PositionError_NED
VelocityError_NED
AccelDesired_NED
ThrustSp
TorqueSp_Body
MotorCmdRaw
MotorCmdSat
SaturationFlag
CommandValid
```

`EstimatorDebugBus` 最小字段：

```text
IMUValid
GPSValid
GPSUpdateApplied
Innovation
CovDiag
EstimatorStatus
IMUTimestamp
GPSTimestamp
```

`TelemetryBus` 最小字段：

```text
StateEstBus
ControlDebugBus
EstimatorDebugBus
HealthStatus
FailureFlag
FailureReason
Mode
```

实施策略：

```text
先用保守字段，宁可少而稳定，不一开始做巨大遥测总线
先暴露已有内部信号，不重写控制律
Telemetry 只出 FlightCore，不进 FlightCore
```

验收标准：

```text
FlightCore 顶层输出 EscCmdBus / StateEstBus / TelemetryBus
TelemetryBus 能解释 motor_cmd 的来源
TelemetryBus 能解释 EKF 是否进行了 GPS 更新
TelemetryBus 不包含 AirSim 专用字段
```

---

## 6. 阶段 P3：ShadowFrameV2 / ExperimentTrace

目标：把一次实时联调变成可分析数据，而不是只看状态行。

修改范围：

```text
3_Integration/SimAdapter/ShadowOutput                         扩展或新增 TraceOutput
3_Integration/RuntimeBridge/protocol/shadow_frame_v2.py       新增
3_Integration/RuntimeBridge/adapters/shadow_logger.py         兼容 v1/v2
3_Integration/RuntimeBridge/adapters/airsim_realtime_bridge.py 使用 v2 CSV
python_analysis/analyze_shadow_trace.py                       新增
```

`ShadowFrameV2` 最小内容：

```text
header:
  magic
  version
  source_id
  sequence
  sim_time_s
  wall_time_s

command:
  mode
  position_sp
  velocity_sp
  yaw_sp
  valid

truth:
  position_ned
  velocity_ned
  valid

estimate:
  position_ned
  velocity_ned
  attitude_quat
  status

control:
  motor_cmd
  saturation_flag
  thrust_sp
  torque_sp

estimator:
  imu_valid
  gps_valid
  gps_update_applied

health:
  failure_flag
  failure_reason
```

兼容策略：

```text
保留 ShadowFrame v1 decoder
新增 v2 magic/version
logger 自动识别 v1/v2
Actuator 写回初期仍可只依赖 v1 所需 motor_cmd/valid
```

验收标准：

```text
Shadow 模式 CSV 每行能同时包含 command / truth / estimate / control / status
可以离线计算 truth-estimate error
可以离线计算 tracking error
可以统计饱和率和传感器有效率
```

---

## 7. 阶段 P4：最小实验分析系统

目标：让数据自动变成指标和图，而不是人工看 CSV。

新增目录：

```text
experiments/
logs/
python_analysis/
```

首批脚本：

```text
python_analysis/parse_shadow_trace.py
python_analysis/compute_metrics.py
python_analysis/plot_episode.py
python_analysis/compare_runs.py
```

首批指标：

```text
position_error_max
position_error_rms
velocity_error_rms
estimation_position_error_rms
motor_saturation_rate
imu_valid_rate
gps_valid_rate
gps_update_rate
shadow_frame_rate
shadow_age_max
failure_flag
failure_reason
```

首批实验：

```text
airsim_shadow_hover
airsim_velocity_step
airsim_sensor_dropout_shadow
airsim_actuator_dryrun
```

验收标准：

```text
运行一次 shadow 实验后，自动生成 trace.csv / summary.json / plots
summary.json 能说明本次实验是否通过
两次实验可以 compare
```

---

## 8. 阶段 P5：FlightCore 内部行为修正

目标：在有观测能力后，再修控制和估计的关键行为。

优先修正：

```text
EKF GPS update 必须由 Valid + Timestamp/Sequence 触发
IMU invalid 时预测路径必须可解释
StateEstBus.Status 不能只等于 gps_valid
EscCmdBus 或 ControlDebugBus 必须表达饱和状态
CommandFrame timeout_s 必须影响 CommandValid 或 HealthStatus
```

验收标准：

```text
关闭 GPS 时，Telemetry 能显示 GPS invalid 且 gps_update_applied=false
重复 GPS timestamp 时，不发生重复更新
命令超时时，CommandValid/HealthStatus 能反映异常
电机饱和时，CSV 和 Telemetry 都能看到
```

---

## 9. 阶段 P6：Actuator 模式小步闭环

目标：在 Shadow 可解释之后，再让 FlightCore 写回 AirSim。

顺序：

```text
1. Shadow only，记录 FlightCore motor_cmd
2. Actuator dry-run，只做映射和日志，不写 AirSim
3. 单轴/单电机 PWM 映射验证
4. 低油门响应验证
5. 姿态稳定
6. 悬停
7. 速度指令
8. 位置保持
```

Actuator 必须额外记录：

```text
motor_cmd_before_mapping
motor_cmd_after_mapping
motor_order
actuator_write_count
actuator_timeout_count
last_shadow_age
AirSim actual response
```

安全边界：

```text
shadow_age 超时 -> 写零 PWM
Telemetry invalid -> 写零 PWM
FailureFlag -> 写零 PWM 或降级
人工 Ctrl+C 必须立即停写
```

---

## 10. 阶段 P7：任务与安全骨架

目标：让飞控基座开始具备任务智能和世界模型标签来源。

最小模式：

```text
DISARMED
ARMED
TAKEOFF
HOLD
VELOCITY_CMD
LAND
FAILSAFE
```

最小故障：

```text
SENSOR_TIMEOUT
COMMAND_TIMEOUT
ESTIMATOR_INVALID
MOTOR_SATURATION_PERSISTENT
POSITION_ERROR_DIVERGING
ATTITUDE_ABNORMAL
RUNTIME_BRIDGE_TIMEOUT
MANUAL_ABORT
```

这些不是世界模型的替代品，而是未来世界模型的训练标签、校准对象和安全外壳。

---

## 11. 优先级排序

立即做：

```text
P1 RuntimeTruthBus
P2 FlightCore TelemetryBus
P3 ShadowFrameV2
P4 最小分析脚本
```

随后做：

```text
P5 EKF/控制器内部行为修正
P6 Actuator 小步闭环
P7 任务与安全骨架
```

暂缓：

```text
更换仿真器
深入调控制器参数
PX4 深集成
Isaac/Pegasus 深集成
RL / Dreamer / 世界模型训练
真实硬件飞行
```

暂缓不是否定，而是等实验基座有能力评估它们。

---

## 12. 完成定义

这个开发计划完成后，系统应能在一次 AirSim Shadow 或 Actuator 实验后自动回答：

```text
输入是什么？
真值是什么？
估计是什么？
目标是什么？
控制器为什么这样输出？
误差如何变化？
是否饱和？
是否延迟？
是否失效？
失败原因是什么？
这次修改是否真的更好？
```

达到这个状态，才算飞控基座从“能联调”进入“能研究”。
