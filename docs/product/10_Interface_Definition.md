# FlightCore — 10 Interface Definition（接口定义）

## 文档信息

| 项目 | 内容 |
|---|---|
| 文档名称 | FlightCore Interface Definition（接口定义） |
| 版本 | v0.1 |
| 状态 | **Draft（A2 契约全部关闭；字段级定稿 2026-08-07）** |
| 来源 | `process/01_Session_and_Decision_Log.md` §6/§13/§13.x（DEC-072/074/081..083/085..090） |
| 上游文档 | 07_System_Requirements；09_System_Logical_Architecture |
| 维护规则 | 信号定义源 = `FC_SimulinkProject/1_Data_Dictionaries/BusConfig/config_*.m`（选择性复用 DEC-084）；字段级明细（单位/类型/序列化/取值范围）在此维护 |

## 1. 任务计划接口（A1：GCS ↔ FlightCore）

### 1.1 语义空间（全语义集，参考 DJI WPML，非照抄格式）

```
航点 = {
  位置 + 高度（参考系可配置：相对声明参考点 / 绝对高程 / AGL——均 GCS 换算，FlightCore 单一参考系）
  速度（任务级默认 + 单点覆盖）
  爬升/下降率（任务级默认 + 单点覆盖；垂直剖面由 FC 制导生成，爬升方式 = 行为参数；DEC-077/078）
  过点方式 = { 直线/曲线 } × { 停/过 }  [+ 协调转弯]
  偏航 = 飞机航向 { 跟随航线（默认，无参）/ 固定航向（覆盖，+角度）}（DEC-081：指向POI 移除 → 挪为云台动作，后置 DEC-009）
  载荷 = 云台（三轴光电吊舱，未来）
  动作 = { 触发条件(reachPoint/定时/距离), 动作序列(takePhoto/gimbalRotate/...) }
  其他：isRisky 风险标记（暂缓）
}

任务级配置 = { finishAction, 链路丢失默认行为, 安全起飞高度, 去首点速度, 默认爬升/下降率, 坐标参考系, 载荷信息 }
```

**接口通用模式（DEC-078）：** 航点级飞行行为参数统一"任务级配置默认 + 航点级单点覆盖"（速度 / 爬升下降率 / 到达判据已定；偏航候选）；结构性字段（位置/高度/过点方式/动作）按航点声明、不适用默认+覆盖。

### 1.2 计划 schema 顶层骨架（TBD-008 已收口，DEC-082/083）

```
MissionPlan {
  meta: { 任务 ID, 版本, 生成时间 }              ✅ 已定（2026-08-07）
  coordinate: { 参考系, 参考点(锚定), 高度参考=相对声明参考点 }
  config: { finishAction, 链路丢失默认, 安全起飞高度, 去首点速度,
            默认速度, 默认爬升率, 到达判据默认, 默认偏航=跟随航线(无参),
            航点数上限 = 400 }                    # DEC-081/083
  waypoints[ N ]: [ Waypoint ]
}
Waypoint {
  position { 经纬度, 高度 }
  speed / climb_rate             # 覆盖，缺省=config 默认（DEC-078）
  pass_mode { line|curve } × { stop|flyover }
  yaw { mode, param }            # 覆盖=固定航向+角度；缺省=跟随航线（DEC-081）
  arrival 判据覆盖                # 缺省=config 默认（DEC-055）
  actions[ { trigger, sequence } ]
  payload 云台（预留，后置）
}
```

**首/末航点语义（DEC-082）：** 首航点 = waypoints[0]、末航点 = waypoints[N-1]（位置隐式，无 role 字段）；末航点"停"由计划校验强制（SYS-REQ-001）。speed/climb_rate 作用于到达该点的航段（首航点到达段 = 去首点速度 config）；固定航向到达该点时调整（DEC-047）。

**容量（DEC-083）：** 单航线航点数接口上限 = **400 航点（暂定）**——需求侧粗估 ~100~300，接口上限含余量。

### 1.3 第一阶段实现 vs 暂缓

**实现：** 位置/高度（相对声明参考点）、速度（全局+单点）、直飞 停/过、**曲线过点（DEC-038）**、固定航向 + 跟随航线、动作（reachPoint 触发）、任务级配置（finishAction/链路丢失默认机制/安全起飞高度/去首点速度）。**断点续飞：接口已定义、实现后置（DEC-037）。**

**暂缓（接口已定义、未实现）：** POI、协调转弯、云台动作（gimbalRotate/录像/变焦）、定时/距离触发、isRisky、地形跟随、绝对高程（EGM96）。

### 1.4 传输（DEC-016）

MAVLink 承担遥测/指令/健康/文件传输；任务计划 = 自研富语义格式，经 MAVLink FTP 上传，FlightCore 的 Mission Manager 解析 + 校验后执行。

## 2. 接口机制（DEC-072/074）

模块间接口采用**直连端口总线** vs **订阅发布**的判据 = **时序要求**，非消费者数量：

| 机制 | 适用 | 链路 |
|---|---|---|
| 直连端口总线 | 强时序（确定周期/低延迟/有界/可验证）——控制环链路 | StateEstBus→FC、TrajectorySetpointBus→FC、EscCmdBus→电机 |
| 订阅发布 | 无严格时序要求（可容忍到达/采样时间不确定） | 遥测/日志/状态回报/诊断/命令事件/航线数据/运动学合同 |

各链路时序强弱由实时性分析（DEC-056）产出后逐条确认；控制环链归属强时序无争议。**控制环直连总线不并入状态流体系**（DEC-087）；任何控制/判据不得依赖状态流。

## 3. A2 契约清单（现有 Bus 复用评估，DEC-084）

> 代码处置姿态 = 选择性复用：执行链信号定义与基线对齐者复用、违反新规则处按基线修正；接口机制按 DEC-072 两分落地。

| 现有 Bus | 契约（会话·决策日志） | 判定 | 接口机制 |
|---|---|---|---|
| TrajectorySetpointBus | Navigator→FC 轨迹参考（DEC-063） | ✅ 复用（Valid 语义已钉，DEC-086） | 直连（强时序）；Validity 经回报进命令审查（订阅） |
| StateEstBus | SE→FC 估计状态（DEC-073） | ✅ 复用 | 直连（强时序） |
| NavigationContractBus / NavigationObjectiveBus | MM→Navigator 运动学合同（DEC-061/062） | ⚠️ **修正复用（DEC-085）**：以 ContractBus 为底扩为"段=run"（约束点序列 ≤64、删判据字段）；ObjectiveBus 废弃 | 订阅发布（段边界事件） |
| NavigationStatusBus | Navigator 状态流（制导状态流；正常流回报已删除 DEC-062） | ❌ **重构**（正常流回报删除、转模块状态流） | **订阅发布**（模块状态流，DEC-087） |
| MissionPlanBus | Gateway→MM 航线数据（DEC-070） | ❌ **重构**（容量 10→400、字段对齐 A1 schema） | 订阅发布（数据通道） |
| CommanderRequestBus | 命令请求（DEC-066） | ⚠️ 复用+扩展（请求集对齐 DEC-066） | 订阅发布（命令事件） |
| CommanderStatusBus | Commander→MM 授权边界 | ✅ 复用 | 订阅发布（状态回报） |
| MissionControlRequestBus | Commander→MM 操作请求 | ✅ 复用 | 订阅发布（命令事件） |
| MissionPlanAvailableBus | 计划可用通知 | ✅ 复用 | 订阅发布（事件） |
| GcsLinkStatusBus | 链路状态 | ✅ 复用 | 订阅发布（遥测） |
| MissionStatusBus | MM 任务状态 | ✅ 复用 | 订阅发布（状态回报） |
| RouteWindowBus | **MM 内部**滑动窗口（Prev/Cur/Next，含 IsFirst/Last） | 保留内部（不跨模块） | — |
| GPS_BUS / IMU_BUS | 传感器测量 | ✅ 复用 | GPS 中速 / IMU 高速直连 |
| DynamicModelBus | 动力学真值 | ✅ 复用 | MIL plant 输出 |
| EscCmdBus | FC→电机指令（DEC-064） | ✅ 复用 | 直连（执行链末端） |
| ThrustTorqueBus | 动力→动力学 | ✅ 复用 | plant 路径 |
| FlightCmdBus | 旧统一命令接口 | ❌ **废弃**（被 TrajectorySetpointBus 取代，DEC-063） | — |
| ExperimentTraceBus / RuntimeTruthBus | 日志/真值 | ✅ 复用 | 订阅发布（日志） |

**关键修正点：**
- **判据泄漏修正（DEC-062）**：NavigationContractBus 删 AcceptanceRadius/RequiredStableTime；NavigationStatusBus 正常流回报删除、转订阅发布降级诊断；到达判据由 MM 从计划 + StateEstBus 自判（DEC-055）。
- **计划容量与字段**：MissionPlanBus 重构对齐 A1 schema（400 上限、速度/爬升率/偏航默认+覆盖、到达判据覆盖、曲线过点、动作）。
- **运动学合同定案（DEC-085）**：ContractBus 为底、段=run（约束点序列 ≤64【暂定】）、加载时预切分、删判据字段、ObjectiveBus 废弃。

## 4. A2 契约字段定义（契约 #1..7，定稿）

### 契约 #1 运动学合同 `NavigationContractBus`（v2，DEC-085）

```
NavigationContractBus — 运动学合同（段=run）
{
  ContractId        uint32     — 单调合同关联号
  PlanId            uint32     — 所属计划
  SegmentIndex      uint32     — 段索引（可追溯）
  StartValid        boolean    — false = Navigator 快照当前状态（DEC-065）
  StartPositionNED  single[3]  — 显式段起点（StartValid=true 时有效）
  PassCount         uint8      — 过点个数 n（0 = 纯停段）
  PassPositionNED   single[64×3] — 过点序列（仅前 PassCount 项有效）
  EndPositionNED    single[3]  — 终止停点
  CruiseSpeed       single     — 段巡航速度
  MaxAcceleration   single     — 段最大加速度
  MaxJerk           single     — 段最大加加速度
  TargetYaw         single     — 终止停点目标偏航（DEC-082）
  Valid             boolean    — 合同完整且授权
}
```

**取舍（DEC-085）：** 删判据字段（AcceptanceRadius/RequiredStableTime，判据归 MM）；终止速度恒零故省 TerminalVelocityNED（段必止于停点）；ExitDirection 由 run 几何 + 窗口推导故省；PathType 并入 PassCount + 过点序列；过点偏航 = 跟随航线切线、固定航向仅作用终止停点（DEC-081/082）；数组定长 64【暂定】。

### 契约 #2 轨迹参考 `TrajectorySetpointBus`（DEC-086）

```
TrajectorySetpointBus — Navigator→FlightControl 连续轨迹参考（直连强时序）
{
  Position_NED_SP  single[3]  — 位置参考（NED，m）
  Velocity_NED_SP  single[3]  — 速度参考（NED，m/s；FC 跟踪速度→姿态→内环，不微分位置，DEC-063）
  Yaw_SP           single     — 偏航参考（绝对航向，rad；跟随航线切线 / 固定航向，DEC-081）
  Valid            boolean    — 运行时有效性：false = 无有效已授权参考，FC 即悬停保持（DEC-086，方案 B）
}
```

**决策（DEC-086）：** 无加速度前馈（留控制分析证明必要性）；偏航单量、无偏航率（控制分析项）；坐标系 NED 与契约 #1 一致；Validity 由 Navigator 权威生产——**正常流恒有效（最差=悬停保持）、false 仅由异常引起**；FC 对 false 的处置 = **FailSafe 事件式处置、后置（TBD-014）**；Commander 经状态回报订阅消费 Validity 作命令审查输入（DEC-086④，预防性门，现在实现）。

### 契约 #3 SE→FC 估计状态 `StateEstBus`（DEC-073/072）

```
StateEstBus — SE 全量状态估计输出（直连 FC 强时序）
{
  Position_NED      single[3]  — 位置估计（NED，m）
  Velocity_NED      single[3]  — 速度估计（NED，m/s）
  Attitude_quat     single[4]  — 姿态四元数 [w x y z]
  AngularRate_Body  single[3]  — 机体角速度估计（rad/s）
  Accel_Body        single[3]  — 机体加速度估计（不含重力，m/s²）— 日志/诊断
  GyroBias          single[3]  — 陀螺零偏估计 — 日志/诊断
  AccelBias         single[3]  — 加速度计零偏估计 — 日志/诊断
  Wind_NED          single[3]  — 风速估计（NED，m/s）— 安全层（DEC-021 后置）
  Status            uint8      — 估计器状态（0未初始化/1稳定/2错误）— 可行性回报→Commander（DEC-086④）
}
```

**决策（2026-08-07）：** ① **整包复用**直连 FC——SE 全量输出、FC 按需消费 {位置/速度/姿态/角速度}；② **消费者拓扑**：FC 直连强时序 / MissionManager 订阅（到达判据字段，中速）/ Navigator 订阅（段边界锚点，契约 #4）/ **SE 状态流 = StateEstBus 数据经订阅发布**——Commander 从订阅读 Status（命令审查）、Logging 订阅落盘、Gateway 转发 GCS 遥测（DEC-087）；③ 估计器内部状态（偏置/加速度）与 Wind 属其他消费域，不因此剥离总线。

### 契约 #4 SE→Navigator 当前状态（段边界锚点，DEC-073/074）

```
StateSnapshot — SE→Navigator 段边界锚点（订阅，宽松时序；仅段边界衔接用）
{
  Position_NED  single[3]  — 当前估计位置（NED，m）
  Velocity_NED  single[3]  — 当前估计速度（NED，m/s）
}
```

**决策（2026-08-07）：** **不含偏航**——Navigator = 轨迹生成（制导），yaw 为姿态量、不消费当前机头；yaw 参考是 Navigator 的**输出**（随轨迹参考，DEC-063/073），由几何切线/固定航向参数产出，非输入。**设计后果：** 段边界 yaw 参考允许阶跃，由 FC 偏航控制器在包线内平滑；DEC-065"段间平滑"原则范围 = 位置/速度，不延伸姿态。

### 契约 #5 `MissionPlanBus` 航线数据（DEC-088）

```
MissionPlanBus — Gateway→MissionManager 航线数据（数据通道，订阅发布；DEC-070/071）
{
  Meta        { TaskId, Version, Timestamp }                        — DEC-083
  Coordinate  { Frame, ReferencePoint, HeightRef=相对声明参考点 }     — DEC-014/015
  Config      { FinishAction, LinkLossDefault, SafeTakeoffHeight,
                DepartureSpeed, DefaultSpeed, DefaultClimbRate,
                ArrivalDefault, DefaultYaw=跟随航线, MaxWaypoints=400 }
  Waypoints   Waypoint[400]          # 定长，DEC-083 容量上限【暂定】
}
Waypoint {
  Position        { Lat, Lon, Height }              — 结构性，按航点声明（DEC-078）
  Speed           single — 覆盖，缺省=config（DEC-078/082）
  ClimbRate       single — 覆盖，缺省=config（DEC-077/078）
  PassMode        { line|curve } × { stop|flyover } — 结构性；末航点=stop 由校验强制（DEC-082/048）
  Yaw             { mode, param } — 覆盖=固定航向；缺省=跟随航线（DEC-081）
  Arrival         { pos,vel,alt,yaw } 覆盖 — 缺省=config（DEC-055）
  Actions         Action[N]     — 已定义未实现（DEC-088；trigger=reachPoint 机制【待核 §6】/payload 动作后置）
  Payload         云台预留        — 已定义未实现（DEC-009）
  IsRisky         boolean       — 已定义未实现（暂缓）
  CoordinatedTurn  — 已定义未实现（协调转弯暂缓）
}
```

**决策（DEC-088）：** 全字段占位、接口一次定型；未实现字段置空+标"已定义、未落地"；容量 400 定长数组、内存预算内（DEC-075）。**残留挂账：** 去首点速度与首航点速度覆盖的关系（DEC-078 推论②）。

### 契约 #6 命令/状态通道（DEC-089/090）

```
CommanderRequestBus — GCS 写命令请求（订阅，命令事件；Gateway→Commander）
{ CommandId uint32, Command 枚举{加锁/解锁|开始任务|接管|返航|降落|悬停|临时上传航点|载荷控制(后置)|...}, Params }
CommandAckBus — 命令结果（订阅，命令事件；Commander→Gateway→GCS；亦入 Logging DEC-090）
{ CommandId uint32, Result 枚举{approved|rejected|aborted}, ReasonCode uint16, Message }
CommanderStatusBus — Commander 模块状态流（DEC-087）
{ Armed 枚举{disarmed|armed}, CurrentMode, LastCommandResult 缓存, ... }
```

**决策（DEC-089/090）：** 逐条 Ack；**多事件时序**——同一 CommandId 可多次发布（approved→aborted），GCS 按 CommandId 聚合取最新、CommandId 单调递增，Ack 事件入 Logging 持久化（事后排查）；ReasonCode 集 = 起飞门 7 条逐条 + 阶段合法性 + 参数非法 + 安全否决（后置），粒度服务操作者 HMI（DEC-003），SUBSYS-REQ 展开；查询通道 request-response 独立于命令 Ack（DEC-066 通道②）。

### 契约 #7..N（复用收口）

GPS_BUS / IMU_BUS（传感器测量，GPS 中速 / IMU 高速直连）、DynamicModelBus（动力学真值，MIL plant）、EscCmdBus（FC→电机指令，直连执行链末端，DEC-064 混合器输出）、ThrustTorqueBus（动力→动力学，plant 路径）、ExperimentTraceBus / RuntimeTruthBus（日志/真值，订阅）——全部 ✅ 复用；**FlightCmdBus 废弃**（DEC-063，被 TrajectorySetpointBus 取代）。

## 5. 模块状态流体系（DEC-087）

除 Commander 外每个模块对自身状态/标志位的对外发布 = **每条模块一条"状态流"**（订阅发布，宽松时序）——一次发布、多消费者订阅并按需组合；**不按消费者拆流**。消费者 = Commander（状态视图/命令审查）+ Logging（落盘）+ Gateway（转发 GCS 遥测）。**控制环直连总线不并入状态流体系。** 状态流内容按用途最小拆分、消费者订阅组合。

- 制导状态流 = NavigationStatusBus（承载完整状态含 Valid）
- SE 状态流 = StateEstBus 数据经订阅发布（Commander 从订阅读 Status；控制环仍走直连）
