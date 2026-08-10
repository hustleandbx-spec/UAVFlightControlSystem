# FlightCore 时间契约与外部闭环最小改造执行交接

**状态：** 待执行；Phase 0 契约裁决前禁止修改模型  
**最后更新：** 2026-07-14  
**适用工程：** `D:\Project\UAVSingleFlightControl`  
**主要模型：** `FlightCore_ROS2_loop.slx`、`FlightCore.slx`、`EKF.slx`、`UAV_FlightControl.slx`  
**目标读者：** 下一位负责修改和验证 Simulink 模型的 Agent

---

## 1. 架构裁决摘要

本改造只解决已证实的时间、采样、数据新鲜度和安全输出问题，不借机重构整个飞控。

强制裁决如下：

1. **不新增顶层集中式 `TimingManager`。** Simulink 固定步长与原生多速率调度负责周期任务；局部事件或 Function-Call 只在有明确算法需求时使用。
2. **区分周期任务和异步数据事件。** `ImuIsNew/GpsIsNew` 只表示唯一新测量；控制任务不得由原始 ROS 消息偶然到达节奏隐式决定。
3. **不伪造跨时钟 Age。** 首个 episode 前，安全超时优先使用本地接收年龄 `ReceiveAge`；跨 Windows、WSL、Simulink 的 transport delay 只记录、不参与控制裁决。
4. **EKF 默认采用新 IMU 驱动 predict。** 不在没有数值需求和协方差验证时，以更快周期重复使用同一 IMU 样本。
5. **最小 Supervisor，不建立九状态等待链。** readiness 原因使用并行标志表达；控制状态只保留必要状态，并显式区分 Armed/Disarmed。
6. **FlightCore 与 RuntimeAdapter 各自只有一个职责所有者。** FlightCore 负责产品安全状态和执行器意图；RuntimeAdapter 负责 ROS/UDP 传输、网络 sequence、超时、电机顺序和运行时钟映射。
7. **先修已证实根因，再跑真实 episode。** sequence wrap、跨时钟容差、高级调度和更复杂降级策略必须依据 episode 数据后置冻结。

本次修改不得引入新的 `/uav/*` topic，不得把 AirSim、ROS2、DDS、UDP 或墙钟符号带入 `FlightCore`。

---

## 2. 任务目标与非目标

### 2.1 当前目标

以最小改动建立一套可编译、可运行、可观测的时间契约，使真实 AirSim 外部闭环满足：

- Rate PID 的实际执行周期与离散实现一致；
- 传感器“有可读旧值”和“刚收到新帧”可区分；
- EKF 对每个唯一测量只处理一次；
- 控制和执行器不会无限使用陈旧状态或陈旧命令；
- 未 Armed、未 Ready 或超时情况下不会输出有效非零电机命令；
- Simulink 不因自由运行显著超前于 AirSim 墙钟；
- 从 t=0 保存足以定位首个异常的证据。

### 2.2 非目标

本任务不负责：

- 重新设计姿态、位置或速度控制律；
- 建立自研实时操作系统或内部消息中间件；
- 复制 PX4 的 Commander、uORB 或 WorkQueue 工程实现；
- 将所有任务强制设为同一频率；
- 新增传感器、topic、仿真器适配器或 MAVLink Gateway；
- 在首个有效 episode 前冻结 sequence wrap、跨时钟容差或全部最终任务率。

---

## 3. 已证实事实与根因

### 3.1 当前实测频率与编译周期

| 项目 | 已测值 |
|---|---:|
| Windows endpoint state/IMU | 约 50 Hz |
| Windows endpoint GPS | 约 5 Hz |
| `/uav/sensors/imu` | 约 50 Hz |
| `/uav/sensors/gps` | 约 5 Hz |
| `/uav/cmd/flight` | 约 50 Hz，但模型当前未订阅 |
| `/uav/actuator/esc_cmd` | 约 200–245 Hz |
| endpoint actuator UDP | 本次约 190–267 packets/s |
| EscCmd publisher 数量 | 1 |
| `FlightCore_ROS2_loop` FixedStep | 0.001 s |
| EscCmd Publish 编译周期 | 0.02 s |
| `UAV_FlightControl` 编译周期 | 0.2 s |

这些数字是现状证据，不是目标频率需求。

### 3.2 首个异常位置

```text
t=0.0 s  Torque_SP 约 0，MotorCmd 约 0.58
t=0.2 s  Torque_SP 开始出现小幅导数响应
t=0.4 s  Torque_SP 反号放大，MotorCmd 首次成为 [0.05,0.05,1,1]
t=0.6 s  Torque_SP 达 ±2，MotorCmd 反向成为 [1,1,0.05,0.05]
```

三个速率 PID 当前为：

```text
SampleTime   = -1
FilterMethod = Forward Euler
N            = 100
P/I/D        = 0.15 / 0.02 / 0.005
```

实际离散极点：

```text
z = 1 - N*Ts = 1 - 100*0.2 = -19
```

实测 Q、R 轴相邻拍倍率约 `-19.4`，与理论一致。用同拍 `Thrust_SP/Torque_SP` 重算 Mixer 后，与 SDI MotorCmd 最大误差仅 `7.45e-10`。因此首个异常层级是速率 PID 的离散实现与实际采样周期不匹配，不是 Mixer、ROS、UDP 或 AirSim 执行器。

### 3.3 pacing 反证

开启 pacing 后，唯一 IMU 帧比例恢复正常，但两组运行仍在仿真 `t=0.4 s` 首次饱和。因此：

- PID 离散不稳定是直接根因；
- Simulink/AirSim 墙钟失配是独立放大因素；
- pacing 不能替代 PID 修复，PID 修复也不能替代实时运行约束。

### 3.4 原始证据

- `episodes/20260713_182900_esccmd_timing_evidence/diagnosis.md`
- `episodes/20260713_182900_esccmd_timing_evidence/baseline_manual_run_sdi.mldatx`
- `episodes/20260713_182900_esccmd_timing_evidence/unpaced_2s_sdi.mldatx`
- `episodes/20260713_182900_esccmd_timing_evidence/paced_2s_sdi.mldatx`
- `bridge/windows/endpoint.log`

---

## 4. 第一性原则

### 4.1 只为已知需求增加机制

任何新增模块必须回答：

1. 它解决哪个已证实故障或验收要求？
2. Simulink、现有接口或 RuntimeAdapter 是否已经提供同一机制？
3. 删除它后，哪个测试会失败？

无法明确回答时，不增加该模块。

### 4.2 频率不能互相复制，但必须联合设计

传感器、估计器、控制器、Mixer 和执行器频率具有不同需求来源，但并非彼此无关。最终选择必须联合满足：

- 控制带宽和相位裕度；
- 传感器采样率和带宽；
- 估计器输出率；
- 计算延迟和调度抖动；
- 执行器接口更新率；
- CPU/代码生成目标的 deadline 预算。

禁止以下推导：

- 因 Rate Control 目标较快，未经传感器规格分析就提高 IMU 频率；
- 因当前 endpoint 只有 50 Hz，就永久把 Rate Control 定为 50 Hz；
- 因顶层 FixedStep 为 1 ms，就让所有模块都以 1 kHz 运行；
- 通过重复旧测量、旧 MotorCmd 或更快 Publish 伪造更高算法频率。

### 4.3 原生调度优先

周期任务使用 Simulink 原生多速率调度：

```text
T_task = N_task * TC_BASE_TS
```

其中 `TC_BASE_TS` 是固定步长调度网格，不代表所有任务都按该周期执行。

禁止：

- 顶层集中生成全部控制任务 Tick；
- 同时用 compiled sample time 和手工 Tick 双重调度同一有状态算法；
- 依赖 `SampleTime=-1` 或 `FixedStep=auto` 让关键状态“自动工作”。

允许：

- 对真正由数据驱动的局部算法使用唯一新帧事件；
- 对必须保证执行顺序的局部路径使用经验证的 Function-Call；
- 对周期任务设置明确采样时间，并使用 Rate Transition 连接不同速率。

### 4.4 数据事件不等于基础时钟

```text
ImuIsNew/GpsIsNew     = 唯一新测量事件
周期控制任务           = Simulink 明确采样时间
执行器发布              = 明确接口周期
```

禁止让原始 ROS `ImuIsNew` 驡动整个飞控。允许经过通用 SensorIngress 验证后的新 IMU 事件驱动 EKF predict；是否驱动 Rate Control 必须由控制架构和状态输出语义决定。

### 4.5 单一职责所有权

| 职责 | 唯一所有者 |
|---|---|
| 控制/估计算法执行周期 | FlightCore 模型的显式 sample time 或局部事件契约 |
| ROS 消息解码、QoS、接收时间 | ROS2Adapter / 顶层 runtime wrapper |
| 跨主机时钟映射 | RuntimeAdapter |
| 网络 sequence、UDP 重复/乱序处理 | RuntimeAdapter / endpoint |
| 电机顺序与 AirSim API | endpoint |
| Armed、控制有效性、产品 failsafe 意图 | FlightCore |
| 网络超时后的最终安全输出 | RuntimeAdapter，按 ActuatorOutput 契约执行 |
| wall/sim ratio、UDP 计数 | RuntimeDiagnostics / episode 工具 |

同一 watchdog、sequence 或 timeout 不得在多个层级独立实现并互相覆盖。

---

## 5. 权威边界与待裁决接口

### 5.1 FlightCore 边界

`FlightCore` 只能消费和产生通用 InterfaceContract 语义：

```text
SensorInput + CommandInput + EstimatorInit
  -> FlightCore
  -> ActuatorOutput + Telemetry
```

以下内容不得进入 FlightCore：

- ROS topic、DDS、UDP、AirSim API；
- Windows/WSL 主机身份；
- WallTime、SimWallRatio；
- ActuatorUdpCount、ROS PublishCount；
- runtime queue depth 或 endpoint connection handle。

### 5.2 Phase 0 必须解决的契约不一致

当前 `interface_contract.md` 中 `EscCmdBus` 仅列出 `MotorCmd[4]`，而 `flightcore_runtime_isolation.md` 和本任务安全语义要求 actuator validity。模型修改前必须裁决并更新权威契约与测试。

最低必要 ActuatorOutput 语义：

```text
MotorCmd[4]     normalized 0..1
Armed           boolean
Valid           boolean
```

可选诊断字段如 source timestamp 或 generation 只有在存在明确消费者和测试时才能加入。

必须定义真值表：

| Armed | Valid | RuntimeAdapter 行为 |
|---:|---:|---|
| 0 | X | 立即输出明确的 disarmed 安全值 |
| 1 | 0 | 立即输出明确的 invalid/timeout 安全值，不保持旧命令 |
| 1 | 1 | 使用当前 MotorCmd，并应用边界限幅和电机映射 |

`disarmed 安全值`、`failsafe 安全值` 和 AirSim API 的持续时间/覆盖语义必须通过 P2 API 探针确定，不得默认等同于 `0`、`0.05` 或“保持上次值”。

### 5.3 Command freshness

当前 `FlightCmdBus` 没有 Timestamp/Sequence，不能在 FlightCore 内可靠计算 `FlightCmdAge`。

首个 episode 前采用最小方案：

- 顶层 `ExternalFlightCmdIngress` 使用本地接收时间计算 `ReceiveAge`；
- 超时后将通用 `FlightCmdBus.Valid=false`；
- FlightCore 只使用 Valid 后的命令，不感知 ROS 时间；
- `FlightCmdAge` 仅进入 RuntimeDiagnostics，不进入产品 Bus。

若未来 FlightCore 必须区分命令采样时刻和接收时刻，再通过契约测试提议新增通用时间字段。

---

## 6. 时间语义

### 6.1 三类时间不可混用

```text
SourceSampleTime  传感器或命令在源端的采样/产生时间
LocalReceiveTime  当前 adapter 收到消息的本地单调时间
LogicalTime       Simulink 执行时间
WallTime          主机墙钟，仅用于运行与证据
```

派生量：

```text
ReceiveAge     = LocalNow - LocalReceiveTime
SampleAge      = MappedNow - MappedSourceSampleTime
TransportDelay = LocalReceiveTime - MappedSourceSampleTime
```

规则：

- `ReceiveAge` 可在无需跨时钟同步时用于本地 timeout；
- `SampleAge` 和 `TransportDelay` 只有在映射已验证后才可作为控制判据；
- epoch 尚未统一时，禁止从 Windows timestamp 与 Simulink LogicalTime 直接相减；
- source timestamp 回跳、source restart 和 adapter restart 必须作为不同事件记录。

### 6.2 Sequence 规则

首个 episode 前：

- 记录已有 sequence；
- 相同 sequence/timestamp 不产生第二个 `IsNew`；
- 明确的回跳或 source restart 使数据暂时 invalid，并记录诊断；
- 不在 FlightCore 内冻结网络 sequence 位宽、wrap 比较或跨重启连续性。

首个 episode 后再根据真实 schema 和日志冻结：

- 位宽和 wrap；
- source identity；
- restart/rebase；
- timestamp 与 sequence 冲突时的权威顺序。

### 6.3 外部实时模式

开发期 AirSim 外部闭环顶层使用：

```text
SolverType   = Fixed-step
Solver       = FixedStepDiscrete
FixedStep    = TC_BASE_TS
EnablePacing = on
PacingRate   = 1
```

`SimWallRatio` 只能衡量长期平均推进率，不能单独证明实时性。还必须记录：

- 各关键任务最大/平均执行时间；
- 最大调度抖动；
- missed deadline / overrun 定义和计数；
- ROS 接收队列深度、丢弃和 latest-sample 策略；
- actuator 端到端最大延迟；
- backlog 或 overrun 后的确定行为。

首个 episode 前可暂用稳态平均目标：

```text
0.98 <= Δt_sim / Δt_wall <= 1.02
```

但它是集成观测指标，不是 FlightCore 产品参数，也不能替代 deadline 判据。

---

## 7. 最小目标架构

```text
FlightCore_ROS2_loop                         # Runtime wrapper
├─ ROS2Subscribe
├─ SensorIngress
│  ├─ Decode/Validate
│  ├─ LatestValueLatch
│  ├─ UniqueSampleCheck
│  ├─ IsNew
│  └─ ReceiveFreshness
├─ ExternalFlightCmdIngress
│  ├─ LatestValueLatch
│  └─ ReceiveFreshness -> FlightCmdBus.Valid
├─ FlightCore                                # Product boundary
│  ├─ EKF
│  ├─ MinimalControlSupervisor
│  ├─ Position/Velocity/Attitude/Rate Control
│  ├─ Mixer
│  └─ ActuatorOutput intent
├─ ROS2Publish
└─ RuntimeDiagnostics
```

不存在顶层 `TimingManager`。

### 7.1 SensorIngress

输入：

```text
ImuMsg
GpsMsg
ImuNewRaw
GpsNewRaw
LocalReceiveTime
```

输出：

```text
IMU_BUS
GPS_BUS
ImuIsNew
GpsIsNew
ImuReceiveFresh
GpsReceiveFresh
SensorTimingStatus      # runtime diagnostics
```

行为：

1. 新 sequence/timestamp 到达时原子锁存整帧 Bus；
2. `IsNew` 仅保持一个被消费任务的执行机会，不重复处理旧帧；
3. 重复或明确乱序帧不产生第二次 `IsNew`；
4. Receive timeout 后将对应通用 Bus 的 `Valid=false`；
5. 未验证跨时钟映射前，不向 FlightCore 输出伪造的 source Age。

### 7.2 EKF

首选 V0 调度：

```text
ImuIsNew -> predict 一次，使用该帧明确 dt/积分时间
GpsIsNew -> correction 一次
```

要求：

- 每个唯一 GPS 样本最多 correction 一次；
- IMU 断流后不得无限使用旧测量；
- source dt、过程噪声和协方差传播保持一致；
- timestamp 异常按 degraded/reset 契约处理；
- 现有 EKF Harness 继续通过，或记录经批准的预期变更。

“周期 predict + IMU 零阶保持”不是 V0 默认方案。只有 EKF 数值设计明确要求子步、并完成过程噪声重新离散化和 MIL 验证后才能引入。

### 7.3 控制任务

关键有状态组件必须具有明确采样时间：

```text
TC_POSITION_CONTROL_TS
TC_VELOCITY_CONTROL_TS
TC_ATTITUDE_CONTROL_TS
TC_RATE_CONTROL_TS
TC_MIXER_TS
TC_ACTUATOR_TS
```

要求：

- 不包含意外的 `ClassicPeriodicDiscrete0.20`；
- 不依赖继承采样时间运行；
- 慢到快使用明确 hold，并携带 Valid/Fresh 语义；
- 快到慢由慢任务读取原子快照；
- 所有跨速率状态路径通过编译 sample-time 审计；
- 任务率在控制、传感器、延迟和执行器约束下联合确定。

### 7.4 Rate PID

对 `PID_RateP/Q/R`：

```text
SampleTime = TC_RATE_CONTROL_TS
```

禁止继续使用 `-1`。

若使用 Forward Euler 导数滤波器，稳定性必要条件为：

```text
N * TC_RATE_CONTROL_TS < 2
```

`N*Ts <= 0.5` 只能作为待验证的设计建议，不是无条件强制规则。最终离散方法必须通过频率响应、相位裕度、噪声和闭环 MIL 验证；可以选择经验证的 Backward Euler 或 Trapezoidal，但不得只为绕过检查而切换方法。

PID 必须定义：

- Armed=false 时 reset/track；
- ControlActive=false 时 reset/track；
- StateEst invalid/stale 时 reset/track；
- FlightCmd invalid/stale 时 reset/track；
- 输出饱和后的 anti-windup；
- ACTIVE 进入/退出的 bumpless 行为。

### 7.5 MinimalControlSupervisor

不采用 `WAIT_TIME/WAIT_SENSOR/WAIT_ESTIMATOR/WAIT_COMMAND` 互斥等待链。

最小状态：

```text
INIT
STANDBY
ACTIVE
FAILSAFE
```

并行 readiness：

```text
TimeReady
ImuFresh
EstimatorValid
StateFresh
CommandFresh
ActuatorPathReady
Armed
```

进入 ACTIVE 必须满足全部 readiness 和显式 Armed。任何 readiness 失效时，按经批准的 timeout/hysteresis 退出 ACTIVE。FAILSAFE 是否可自动恢复必须明确，不得由信号偶然恢复隐式决定。

ACTIVE 前：

- ActuatorOutput `Valid=false` 或 `Armed=false`；
- PID 保持 reset/track；
- 不输出有效非零电机命令；
- 不累积积分器。

### 7.6 ActuatorOutput 与 RuntimeAdapter

FlightCore 输出产品语义，RuntimeAdapter 实现传输：

```text
FlightCore: MotorCmd + Armed + Valid
RuntimeAdapter: publish sequence + timestamp + network timeout + motor order
Endpoint: AirSim API + final safe output
```

发布频率由执行器接口需求决定，不由 IMU 频率直接决定。若 publish 快于 Mixer，必须明确是保持最新有效命令；保持不能被记录为一次新的控制计算。

---

## 8. 参数冻结策略

### 8.1 模型修改前必须冻结

| 参数/语义 | 决策依据 |
|---|---|
| `TC_BASE_TS` | 所有批准周期任务的公共调度网格与计算预算 |
| `TC_RATE_CONTROL_TS` | 控制带宽、传感器/状态率、延迟、执行器能力 |
| PID 离散方法与 `N` | 稳定性、噪声和频率响应验证 |
| IMU/GPS receive timeout | 当前链路实测间隔与安全需求，标明临时值 |
| Command receive timeout | 当前命令源实测间隔与安全需求，标明临时值 |
| Armed/Valid 真值表 | 产品安全契约 |
| invalid/timeout actuator 行为 | RuntimeAdapter 与 endpoint 联合契约 |

所有周期任务必须满足：

```text
TC_x_TS / TC_BASE_TS 为正整数
```

该限制只适用于 Simulink 周期任务，不适用于异步消息到达间隔。

### 8.2 首个 episode 后再冻结

- sequence 位宽、wrap 和 restart 语义；
- 三时钟域映射与 transport delay 容差；
- 最终 `TC_IMU_NOMINAL_TS`、`TC_GPS_NOMINAL_TS` 统计容差；
- 最终 jitter、deadline、overrun 阈值；
- 是否需要 EKF 子步；
- 是否需要更复杂 DEGRADED 状态和自动恢复策略；
- 最终 actuator publish/hold 语义优化。

---

## 9. 实施阶段与检查点

### Phase 0：契约一致性与现场基线

在任何 `.slx` 修改前完成：

1. 使用 `model_overview/model_read/model_query_params` 现场确认四个目标模型；
2. 输出当前 compiled sample-time 矩阵；
3. 确认 `0.2 s` 调度来源；
4. 裁决 `EscCmdBus` 的 Armed/Valid 语义并先补契约测试；
5. 裁决命令 freshness 在顶层 ingress 计算；
6. 定义 timeout 后 RuntimeAdapter/endpoint 的确定安全输出；
7. 保存未修改模型的基线证据。

**通过条件：** 用户批准最小参数表和安全真值表；契约与测试先于模型变更。

### Phase 1：修复直接根因

修改范围：

- 顶层 `FixedStepDiscrete + TC_BASE_TS`；
- 开启开发期 pacing；
- 消除 `UAV_FlightControl` 意外 `0.2 s` 继承；
- 为 Rate PID 设置明确 sample time；
- 修复离散导数滤波器；
- 完成 anti-windup 和 reset/track 最小语义。

**通过条件：** t=0–2 s 不再出现约 `-19` 倍反号增长；Rate PID compiled sample time 精确等于批准值。

### Phase 2：最小 Sensor/Command Ingress

修改范围：

- 接出并使用 ROS Subscribe 的新帧指示；
- 唯一帧原子锁存；
- 本地 ReceiveFreshness；
- timeout 后通用 Bus `Valid=false`；
- 外部命令与 `HoverCommand5s` 明确隔离。

**通过条件：** 10 秒内 IsNew 数等于唯一帧数；重复/乱序不重复处理；IMU、GPS、命令断流不无限保留有效状态。

### Phase 3：最小安全门控与 actuator 契约

修改范围：

- `INIT/STANDBY/ACTIVE/FAILSAFE`；
- readiness bitset；
- 显式 Armed；
- ActuatorOutput Valid/Armed；
- RuntimeAdapter timeout 与最终安全值。

**通过条件：** ACTIVE 前无有效非零 actuator 输出；任何关键 freshness 失效后在批准时间内进入确定安全输出；恢复行为符合批准契约。

### Phase 4：真实 AirSim 外部闭环 episode

从全新启动 t=0 同步记录：

```text
EscCmd/MotorCmd
Mixer raw input
Torque_SP/Thrust_SP
姿态与角速度误差
EKF attitude/gyro/state validity
IsNew/Valid/ReceiveAge
各周期任务执行计数和 compiled rate
source/receive/logical/wall timestamps
ROS publish/receive counts
UDP send/receive counts
overrun/deadline diagnostics
```

**通过条件：** 真实 AirSim 外部闭环连续运行 30 秒，证据可回放；Torque 不离散发散；MotorCmd 不在 `0.05/1` 间交替；ACTIVE 前无有效非零输出；超时不保持无限陈旧命令。

### Phase 5：依据 episode 固化 V0 时间契约

依据 Phase 4 数据更新：

- sequence wrap/restart；
- 三时钟映射与容差；
- jitter、deadline、overrun；
- 最终 freshness 阈值；
- 最终控制/估计/执行器率；
- 是否需要额外 DEGRADED 策略或 EKF 子步。

**通过条件：** 所有冻结值有证据来源、单位、消费者和自动化测试。

---

## 10. 验证计划

### 10.1 静态与编译检查

- [ ] `FixedStep` 不是 `auto`；
- [ ] 关键 PID、滤波器、积分器、Unit Delay 没有 `SampleTime=-1`；
- [ ] compiled sample time 只包含批准的周期集合；
- [ ] 所有跨速率有状态路径使用明确 Rate Transition/hold；
- [ ] 没有集中式 `TimingManager` 或重复调度；
- [ ] FlightCore 不包含 ROS2/DDS/UDP/AirSim/WallTime 符号；
- [ ] 只有一个 EscCmd publisher；
- [ ] Bus 与权威 InterfaceContract 一致。

### 10.2 组件 MIL

#### SensorIngress

- 唯一帧产生一次 IsNew；
- 重复、乱序、timestamp 回跳和 source restart；
- Bus 原子锁存；
- receive timeout 后 Valid=false；
- 未同步时不伪造跨时钟 Age。

#### EKF

- 每个 IMU唯一帧 predict 次数正确；
- 每个 GPS唯一帧最多 correction 一次；
- IMU断流按契约停止/降级/reset；
- dt、过程噪声和协方差传播一致。

#### Rate PID

- 零误差和小噪声下 Torque 有界；
- 不出现固定约 `-19` 倍反号增长；
- 饱和时 anti-windup 生效；
- 解饱和后在批准时间内恢复；
- Armed/Active/State/Command 失效时 reset/track 正确。

#### Supervisor/Actuator

- readiness 任意组合均不会误进入 ACTIVE；
- 未 Armed 永不输出有效非零命令；
- timeout 行为不依赖旧命令；
- ACTIVE 进入/退出 bumpless；
- FAILSAFE 恢复或锁存语义明确。

### 10.3 集成 MIL

- 多速率 Rate Transition 数据完整性；
- EKF 到控制器的 StateEst 快照一致性；
- 外部命令与内部测试命令隔离；
- ROS publish hold 不伪装成新控制计算；
- pacing 开/关对控制数值结果的影响可解释；
- average ratio、jitter、deadline 和 overrun 同时记录。

### 10.4 故障测试

- AirSim reset/time jump；
- IMU、GPS、FlightCmd 断流；
- source/adapter restart；
- actuator UDP 中断；
- ROS 重复 publisher 或消息 backlog；
- pacing 失败；
- Simulink 任务 overrun；
- Armed/Disarmed 和 FAILSAFE 边界切换。

---

## 11. 诊断分层

### 11.1 FlightCoreDiagnostics

只包含产品语义：

```text
EstimatorValid
StateFresh
ControlState
Armed
ControllerResetStatus
SaturationStatus
AlgorithmExecutionCounts
ActuatorIntentValid
```

### 11.2 RuntimeDiagnostics

只存在于 runtime wrapper、adapter、endpoint 或 episode 工具：

```text
SimTime
WallTime
SimWallRatio
SourceSampleTime
LocalReceiveTime
ReceiveAge
ClockOffsetEstimate
ROS receive/publish counts
UDP send/receive counts
queue depth/drop count
deadline miss/overrun
```

不得为了构造一条“大而全”的 TimingDiagnosticsBus 而把 RuntimeDiagnostics 反向送入 FlightCore。

---

## 12. API 与工具验证待办

在实际编辑前必须现场验证：

- ROS2 Subscribe block `IsNew` 的精确定义、pulse duration 和 sample-time 行为；
- ROS2 Publish block 在 hold/enable 条件下是否重复发布；
- Model Reference 多速率编译行为；
- PID Controller 在 MATLAB R2025b 中各离散方法、reset 和 anti-windup 的实际参数名；
- AirSim `moveByMotorPWMsAsync` 是覆盖旧命令还是排队，以及 duration 语义；
- endpoint 提供的唯一 IMU timestamp/sequence 频率，而不只是 API 轮询次数；
- RuntimeAdapter 的 ROS queue depth、drop 和 latest-sample 行为；
- invalid/disarmed actuator frame 到最终电机安全值的完整路径。

API 行为未验证前，不得将假设写成架构事实。

---

## 13. 明确禁止事项

- 禁止新增集中式 `TimingManager`；
- 禁止把所有任务频率简单设为相等；
- 禁止重复旧传感器数据伪造高频测量；
- 禁止用更快 ROS Publish 重复旧 MotorCmd 伪造高频控制；
- 禁止用 source timestamp 与未映射 LogicalTime 直接计算 Age；
- 禁止让 FlightCore 读取 WallTime、UDP计数或 ROS queue 状态；
- 禁止 Simulink 和 RuntimeAdapter 各自维护冲突的 actuator timeout；
- 禁止 pacing 被当作 PID 修复；
- 禁止 PID 修复被当作忽略墙钟和 deadline 的理由；
- 禁止在未 Armed 时因 readiness 全部为真而自动输出电机命令；
- 禁止把无 ROS 的内部 `HoverCommand5s` wind-up 与外部闭环 t=0.4 s PID发散混为同一问题；
- 禁止以离线四电机一致输出代替真实外部闭环验证；
- 禁止在用户批准 Phase 0 前保存结构性 `.slx` 修改。

---

## 14. Definition of Done

- [ ] 权威契约明确 ActuatorOutput 的 MotorCmd/Armed/Valid；
- [ ] 命令 freshness 的本地接收时间所有者明确；
- [ ] 顶层使用明确固定步长和开发期 pacing；
- [ ] 无集中式 TimingManager；
- [ ] 所有关键有状态任务具有明确 compiled sample time；
- [ ] SensorIngress 对唯一帧正确产生 IsNew，并在 timeout 后 invalid；
- [ ] EKF predict/correction 次数符合 V0 事件契约；
- [ ] Rate PID 离散实现与实际周期稳定匹配；
- [ ] Torque 不再约 `-19` 倍反号放大；
- [ ] Mixer/MotorCmd 不再发生 `0.05/1` 高频翻转；
- [ ] 外部模式不再使用默认 `HoverCommand5s`；
- [ ] 未 Armed、未 Ready 或 timeout 时无有效非零 actuator 输出；
- [ ] RuntimeAdapter 不无限保持陈旧 actuator 命令；
- [ ] FlightCoreDiagnostics 与 RuntimeDiagnostics 已分层；
- [ ] 真实 AirSim 外部闭环 30 秒 episode 可回放、可比较、可判据化；
- [ ] sequence、时钟容差、deadline 和 freshness 最终值均有 episode 证据与测试。

---

## 15. PX4 参考结论

参考 PX4 的原则，不复制其具体实现：

1. PX4 将模块组织为独立 task/work-queue item，可由固定间隔或 uORB 更新回调调度；不存在飞控顶层广播全部控制 Tick 的产品级 `TimingManager`。
2. Multicopter Rate Control 由角速度更新回调触发，并使用 `timestamp_sample` 计算受限 dt；这说明数据事件可以驱动局部算法，但不等于原始 ROS IMU 应成为整个系统时钟。
3. EKF2 由 IMU/`sensor_combined` 更新触发，并使用传感器积分时间；这支持 V0 采用唯一新 IMU 驱动 predict。
4. PX4 将 arming、failsafe action 和 actuator safe output 明确分层；本项目不复制完整 Commander，但必须具备最小 Armed/Valid/timeout 真值表。
5. PX4 的 uORB 是内部异步事实层；本项目当前不提前建设完整内部消息中间件，而使用稳定 InterfaceContract、原子快照和显式 Rate Transition 保持同一分层原则。

参考：

- PX4 Architectural Overview: <https://docs.px4.io/main/en/concept/architecture>
- PX4 Work Queue Module Template: <https://docs.px4.io/main/en/modules/module_template>
- PX4 Multicopter Rate Control: <https://github.com/PX4/PX4-Autopilot/blob/main/src/modules/mc_rate_control/MulticopterRateControl.cpp>
- PX4 EKF2: <https://github.com/PX4/PX4-Autopilot/blob/main/src/modules/ekf2/EKF2.cpp>
- PX4 Arm/Disarm: <https://docs.px4.io/main/en/advanced_config/prearm_arm_disarm>
- PX4 Flight Termination: <https://docs.px4.io/main/en/advanced_config/flight_termination>

---

## 16. 下一位执行 Agent 的第一轮动作

1. 读取项目 `AGENTS.md`、`CLAUDE.md`、README、开发计划、权威契约和本文件；
2. 检查 worktree，保留所有既有用户修改；
3. 启动/连接 MATLAB R2025b，执行 `satk_initialize` 后再执行 `validate_installation`；
4. 使用 `model_overview/model_read` 重新确认四个目标模型；
5. 查询所有关键块 compiled sample time，输出现状矩阵；
6. 完成 Phase 0 的 ActuatorOutput、Command freshness 和 timeout 安全真值表；
7. 先补契约测试并提交用户审核；
8. 未经用户批准，不开始模型结构修改。
