# FlightCore — 05 Development Plan（第一阶段开发计划）

## 文档信息

| 项目 | 内容 |
|---|---|
| 文档名称 | FlightCore Development Plan（第一阶段开发计划） |
| 版本 | v0.1 |
| 状态 | **Draft（2026-08-12 建立，SE 链第一阶段闭合后的实施入口）** |
| 角色 | 过程链槽位（ISO 15288 项目规划 / Project Planning）；第一阶段 MIL+SIL 开发的**唯一执行入口** |
| 上游文档 | 07_System_Requirements（SYS-REQ-001..022+014a）；10_Interface_Definition（A1+A2 契约）；14_SUBSYS_REQ（46 条）；13_V&V_Plan（VER-001..022）；process/01（DEC-001..123）；process/02（R-01..07） |
| 自包含承诺 | 本计划内联第一阶段开发所需的**范围 / 模块职责 / 接口契约字段 / 关键决策 / 实施顺序 / 验收判据 / 数值回填来源 / 风险**；按本计划即可执行，无需回查其他前置文档。深层细节可追溯（引用 ID），但每个工作包自带规格与验收判据 |
| 维护规则 | 里程碑完成→同步更新本计划状态与门禁记录（process/04）；决策变更→process/01（DEC 顺延）；风险→process/02 |

---

## 1. 目标与范围（第一阶段）

**一句话目标：** 在 MIL + SIL 两级纯仿真中，让 FlightCore 正常流全流程（起飞→出航→任务→返航→降落→触地）按要求跑通并完成 V&V 验证（VER-001..022），形成可复现、可追溯、判据明确的开发交付（DEC-033/114）。

### 1.1 范围内（本计划覆盖）

- **7 个模块**：Gateway / Commander / MissionManager / Navigator（制导）/ FlightControl / StateEstimation / Logging（DEC-074）。
- **正常流全流程**（DEC-111）：五段自主执行 + 操作者介入（远程模式命令 / 接管）+ 起飞门一次性审查 + 命令审查拒绝（正常流）。
- **两级验证**：MIL（模型在环）→ SIL（代码生成后在环）（DEC-114）。
- **验证环境**：单一仿真环境（13 §2.3）——plant 基线 + MAVProxy（开发期 GCS，成熟期转 QGC）+ 场景编排注入位 + 数据记录可复现。

### 1.2 范围外（实现后置，**本计划不开发**）

| 后置项 | 依据 |
|---|---|
| SYS-REQ-012/013/014a（异常告警 / 能源中断处置 / 门失效 abort） | DEC-033/111，异常工作包 TBD-003/014 |
| FailSafe 处置机制 | TBD-014（DEC-091/111） |
| 运行时参数/配置系统 | TBD-015（DEC-112） |
| 动态禁飞区强制、感知型约束（避障/SLAM/地形跟随） | DEC-006/025/033 |
| 断点续飞 Resume、异地起降、多计划 | DEC-037/040/016（TBD-016） |
| A1 暂缓字段落地：POI / 协调转弯 / 云台动作 / 定时/距离触发 / isRisky / 地形跟随 / 绝对高程 | 10 §1.3 |
| TPM、实机 Validation、HIL | TBD-001 / DEC-114（第二阶段） |
| 高级算法（LQR/MPC/RL）落地 | A-007（第一阶段仅 MIL/SIL 试验） |

> 后置项接口/需求均已定义（DEC-008 全语义集纪律），仅不实现；本计划不得为赶进度提前实现后置项。

---

## 2. 已闭合前置（SE 链状态，**无需再访谈**）

- **需求链**：SN-001..011 → MOE-A/B/C/D + 安全门 → SYS-REQ-001..022+014a（010/011 已定稿 DEC-110/121）→ SUBSYS-REQ 46 条（双向追溯）。
- **架构**：功能/逻辑/物理收口（DEC-057..075）；模块集 + 接口机制（直连 vs 订阅，DEC-072）+ 物理分配（DEC-075）。
- **接口**：A1 任务计划 schema + A2 契约 #1..7（21 条 Bus，见 §4）——BusConfig 定义层已落地，L1 批测 21/21 PASS，GlobalTypes.sldd 重建（L2）完成。
- **V&V**：VER-001..022 矩阵 + VCRM 框架 + 验证环境四件套 + Validation 推理链（13，2026-08-12 完成）。
- **门禁**：G-001..004 已过签。**G-004 遗留 = L3 模型字段对齐**（Navigator/Commander/CommandGenerator 仍引用已删/改名字段，编译即失败）→ 本计划 M2 承接。
- **下一步 = 执行层**（非访谈）：行为测试对齐 → L3 模型重构 → 数值回填 MBD → VER 执行。

---

## 3. 模块集与职责（自包含摘要）

> 完整需求文本见 14_SUBSYS_REQ.md（SUBSYS-REQ-{MM,CMD,NAV,SE,FC,GATE,LOG}）。下表为执行所需职责摘要 + 关键决策 + 主接口 + 对应 VER。

### 3.1 MissionManager（MM）— 任务语义唯一权威
- **职责**：接收航线数据（数据通道）；维护单一计划槽（覆盖/失败保留/飞中拒绝）；解析+结构校验（不含包线/禁飞区语义）；段（run）预切分查表推进；下发运动学合同给 Navigator；判定停点到达与推进；末航点→返航（默认返航生成）；降落意图→触地判定→任务结束上报；消费 Commander 模式级命令（立即中止计划）；发布任务状态流；临时上传航点（go-to-point）。
- **关键决策**：DEC-085（运动学合同 v2）、DEC-092/093（校验范围/单计划槽）、DEC-095（到达判据连续 N 周期）、DEC-094/096/098/099（停点/返航/规划层）、DEC-100（触地多判据）、DEC-101/102/106/108/109/110。
- **SUBSYS-REQ**：MM-001..020。
- **主接口**：MissionPlanBus（收）、NavigationContractBus（发）、MissionStatusBus（发）、MissionPlanAvailableBus（发）、MissionControlRequestBus（收）、RouteWindowBus（内部）。
- **相关 VER**：001/002..009/010/011/014/015/017/019/020/021。

### 3.2 Commander（CMD）— 统一命令门卫 + 状态审查
- **职责**：GCS 写命令统一入口与路由；命令审查（程序/状态合法性 + 可行性标志 + "开始任务"7 门一次性检查）；持有武装状态（加锁/解锁）；订阅全模块状态流维护只读审查视图；逐条 CommandAck 回报；发布自身状态。
- **关键决策**：DEC-058/059/060（门卫/审查/状态视图只读镜像）、DEC-066（三通道）、DEC-089/090（逐条 Ack + 多事件时序）、DEC-103（解锁判据）、DEC-111（第一阶段无异常处置，仅正常流预防）。
- **SUBSYS-REQ**：CMD-001..006。
- **主接口**：CommanderRequestBus（收）、CommandAckBus（发）、CommanderStatusBus（发）、MissionControlRequestBus（发，路由后）、各模块状态流（订阅）。
- **相关 VER**：002..009/011。

### 3.3 Navigator（制导，NAV）— 统一轨迹生成器
- **职责**：唯一运动轨迹生成器（航线段/起飞爬升/默认返航/降落剖面均 Navigator 生成）；按运动学合同整段规划 + 段内确定性求值；输出连续轨迹参考 {位置,速度,偏航,Valid}；悬停保持；发布制导状态流；不解析任务语义、判据无感知。
- **关键决策**：DEC-104/107（统一轨迹生成器）、DEC-065（段边界锚定当前状态）、DEC-073（合同求值率）、DEC-086（Valid 语义=方案 B，正常流恒有效）、DEC-081/082（偏航）、DEC-062（判据无感知）。
- **SUBSYS-REQ**：NAV-001..005。
- **主接口**：NavigationContractBus（收）、TrajectorySetpointBus（发，直连 FC）、NavigationStatusBus（发）、StateSnapshot（收，段边界）。
- **相关 VER**：015/017/018/019/020。

### 3.4 StateEstimation（SE）— 状态估计
- **职责**：估计 {位置,速度,姿态,角速度}；经 StateEstBus 全量直连 FC；发布状态快照给 Navigator（段边界）；状态流经订阅供 Commander/Logging/Gateway；测量输入 RTK(定位+定向)/IMU/气压；预留测量源扩展位。
- **关键决策**：DEC-073/072（直连强时序）、DEC-105（输入接口扩展）、DEC-068（"导航精度"=SE 误差）、DEC-087（状态流）。
- **SUBSYS-REQ**：SE-001..005。
- **主接口**：StateEstBus（发，直连）、StateSnapshot（发，订阅）、SE 状态流（订阅发布）。
- **相关 VER**：003/010（集成）。

### 3.5 FlightControl（FC）— 控制律执行
- **职责**：闭环跟踪轨迹参考（速度→姿态→角速率内环级联）；执行链多级结构性包线饱和；虚拟控制量 {F,Mx,My,Mz} 混合分配为电机指令（EscCmdBus）；触地后电机停转。
- **关键决策**：DEC-063（跟踪速度不微分位置/无加速度前馈）、DEC-077（多级饱和点 + 双角色）、DEC-064（混合器在 FC 内、矩阵参数走 Vehicle Def）、DEC-042/100（触地停转）。
- **SUBSYS-REQ**：FC-001..004。
- **主接口**：TrajectorySetpointBus（收）、StateEstBus（收）、EscCmdBus（发）。
- **相关 VER**：017/020/021/022。

### 3.6 Gateway（GATE）— MAVLink 协议层
- **职责**：MAVLink 帧收发 + 子协议状态机（命令 / FTP / Time Sync，参数只读子集）；按三通道模型路由（遥测→GCS、命令/查询→Commander、航线数据→MM）；计划文件 FTP 上传；发布链路状态。不做命令语义判断。
- **关键决策**：DEC-016/069（MAVLink 协议层非转发）、DEC-112（子协议集）、DEC-070/071（数据/指令分两路）。
- **SUBSYS-REQ**：GATE-001..004。
- **主接口**：GcsLinkStatusBus（发）、CommanderRequestBus（发）、MissionPlanBus（发）。
- **相关 VER**：001/008/011。

### 3.7 Logging（LOG）— 日志/诊断（独立横切）
- **职责**：订阅各模块状态流/事件落盘持久化（MOE-A/B 判定依据）；持久化 CommandAckBus 事件（时间线重建）；不做在线日志诊断查询。
- **关键决策**：DEC-069/087（订阅落盘）、DEC-090（命令事件时间线）、DEC-113（非飞行中能力）。
- **SUBSYS-REQ**：LOG-001/002。
- **主接口**：各模块状态流 / 事件（订阅）。
- **相关 VER**：010（MOE-A/B 判定依据）。

---

## 4. 接口契约快照（21 条 Bus，A2 契约 #1..7）

> 字段级明细权威源 = `FC_SimulinkProject/1_Data_Dictionaries/BusConfig/config_*.m` + 10_Interface_Definition §4。下表为执行（尤其 M2 模型对齐）所需字段级摘要。**接口机制 = 直连（强时序控制环）vs 订阅发布（宽松时序），判据 = 时序要求（DEC-072）。**

### 契约 #1 运动学合同 `NavigationContractBus`（MM→Navigator，订阅/段边界事件）
```
{ ContractId uint32, PlanId uint32, SegmentIndex uint32,
  StartValid boolean, StartPositionNED single[3],
  PassCount uint8, PassPositionNED single[64×3],   // 过点序列，仅前 PassCount 有效
  EndPositionNED single[3], CruiseSpeed single,
  MaxAcceleration single, MaxJerk single,
  TargetYaw single, Valid boolean }
```

### 契约 #2 轨迹参考 `TrajectorySetpointBus`（Navigator→FC，直连强时序）
```
{ Position_NED_SP single[3], Velocity_NED_SP single[3], Yaw_SP single, Valid boolean }
```

### 契约 #3 估计状态 `StateEstBus`（SE→FC，直连强时序）
```
{ Position_NED single[3], Velocity_NED single[3], Attitude_quat single[4],
  AngularRate_Body single[3], Accel_Body single[3],
  GyroBias single[3], AccelBias single[3], Wind_NED single[3],
  Status uint8 }   // 0未初始化/1稳定/2错误
```

### 契约 #4 段边界锚点 `StateSnapshot`（SE→Navigator，订阅宽松时序）
```
{ Position_NED single[3], Velocity_NED single[3] }   // 不含偏航（DEC-065/084）
```

### 契约 #5 航线数据 `MissionPlanBus`（Gateway→MM，数据通道订阅；A1 扁平化编码）
```
{ TaskId uint32, Version uint32, Timestamp double,
  Frame uint8, ReferencePointNED single[3], HeightRef uint8,
  FinishAction uint8, LinkLossDefault uint8, SafeTakeoffHeight single,
  DepartureSpeed single, DefaultSpeed single, DefaultClimbRate single,
  ArrivalDefault single[4], DefaultYawMode uint8,
  WaypointCount uint16,
  Lat single[400], Lon single[400], Height single[400],
  Speed single[400], ClimbRate single[400],
  PassMode uint8[400], YawMode uint8[400], YawParam single[400],
  ArrivalPos single[400], ArrivalVel single[400], ArrivalAlt single[400], ArrivalYaw single[400],
  ActionTrigger uint8[400], ActionCount uint8[400], PayloadValid boolean[400],
  IsRisky boolean[400], CoordinatedTurn boolean[400],
  Valid boolean }
```
> 容量 400 定长【暂定】；未落地字段（Action/Payload/IsRisky/CoordinatedTurn）置空 + 标"已定义、未落地"（DEC-088）。

### 契约 #6 命令/状态通道（订阅，命令事件 / 状态回报）
```
CommanderRequestBus { CommandId uint32, Command uint8, Params single[8], Valid boolean }
   // Command: 1=加锁 2=解锁 3=开始任务 4=返航 5=降落 6=悬停 7=接管 8=go-to-point激活 9=载荷控制(后置)
CommandAckBus { CommandId uint32, Result uint8, ReasonCode uint16, Message uint8[32], Valid boolean }
   // Result: 0=已批准 1=已拒绝 2=已中止；ReasonCode = 起飞门7条逐条 + 阶段非法 + 参数非法 + 安全否决(后置)
CommanderStatusBus { Armed boolean, CurrentMode uint8, LastCommandResult uint8,
                     LastCommandId uint32, SafetyState uint8, SafetyDirective uint8,
                     MissionDirective uint8, FailsafeActive boolean, Valid boolean }
MissionControlRequestBus { RequestId uint32, CommandId uint32, Action uint8,
                           TargetPositionNED single[3], Valid boolean }
   // Action: 1=开始任务 2=返航 3=降落 4=悬停 5=接管 6=go-to-point激活
MissionPlanAvailableBus { Available boolean, ReasonCode uint8, PlanId uint32, Valid boolean }
   // ReasonCode: 1=不可解析 2=字段非法 3=容量>400 4=末航点非停 5=段结构非法（不经 CommandAckBus）
```

### 契约 #7 复用收口（BusConfig 已存在）
- **GPSBus / IMUBus**（传感器测量，GPS 中速 / IMU 高速直连）、**DynamicModelBus**（动力学真值，MIL plant 输出）、**EscCmdBus**（FC→电机指令，直连执行链末端）、**ThrustTorqueBus**（动力→动力学，plant 路径）、**ExperimentTraceBus / RuntimeTruthBus**（日志/真值，订阅）。
- **GcsLinkStatusBus**（链路状态，遥测 / 起飞门输入）：`{ SourceId uint32, RxSequence uint32, LastRxTimeSec double, LinkQuality single, Connected boolean, Valid boolean }`。
- **RouteWindowBus**（MM 内部段滑动窗口，**不跨模块**）：`{ PlanId, ItemIndex, PreviousValid, PreviousPositionNED[3], CurrentTaskType, CurrentPositionNED[3], CurrentYaw, AcceptanceRadius, TerminalBehavior, AdvancePolicy, DwellTime, CruiseSpeed, MaxAcceleration, MaxJerk, NextValid, NextPositionNED[3], IsFirstItem, IsLastItem, Valid }`。

### 模块状态流体系（DEC-087，一次发布多消费者订阅；控制环直连不并入）
```
NavigationStatusBus { ContractId uint32, GuidanceState uint8, Valid boolean }   // 0=等待合同 1=段轨迹求值 2=悬停保持 3=降落剖面
MissionStatusBus   { PlanId uint32, PlanValid boolean, ExecutionPhase uint8, MissionOutcome uint8,
                     MissionReason uint8, CurrentItemIndex uint8, ItemCount uint8,
                     ReachedItemIndex int16, ActiveContractId uint32,
                     ExecutionHealthy boolean, MissionFaultCode uint16, LandingCompleted boolean, Valid boolean }
SE 状态流          = StateEstBus 数据经订阅发布（Commander 读 Status；控制环仍走直连）
```

### 接口机制结论（DEC-072/087）
- **直连（强时序）**：StateEstBus→FC、TrajectorySetpointBus→FC、EscCmdBus→电机、GPSBus/IMUBus→SE。
- **订阅发布（宽松时序）**：遥测 / 日志 / 状态回报 / 诊断 / 命令事件 / 航线数据 / 运动学合同。
- 控制环直连总线**不并入**状态流体系；任何控制/判据**不得依赖**状态流。

---

## 5. 实施工作包与里程碑（顺序 = 约束 8 硬性）

> 总顺序：**M1 行为测试对齐 → M2 L3 模型字段对齐 → M3 数值回填 MBD → M4 VER 执行**。不可跳步；每步先改测试再动模型（约束 8：契约变更先测试）。
>
> **当前进度：M1 已完成（2026-08-12）→ 下一步 M2 L3 模型字段对齐。**

### M0 卫生与基线（已完成事项确认，不再执行）
- ✅ 定义层完成：BusConfig 21 条与 A2 契约对齐、GlobalTypes.sldd 重建（21 总线）、L1 批测 21/21 PASS、L2 字典可构建。
- ✅ SE 链闭合：G-001..004 过签；数值【暂定】注册（15 + 本计划 §6）。
- **门禁 G-005 前置**：M1..M4 全部完成。

### M1 行为测试对齐新契约（约束 8 第一优先）——✅ 已完成（2026-08-12）
- **目标**：让 `FC_SimulinkProject/4_Test/features/*.feature` 与 BusConfig 新 schema 一致，先锁定契约再动模型。
- **动作**：按 §4 契约字段重写/校验以下 .feature 的断言：`mission_manager.feature`、`navigator.feature`、`commander_operational_state.feature`、`navigator_stop_and_go.feature`、`mission_manager_structured_objective.feature`、`mission_manager_land_completion.feature`、`mission_manager_commander_integration.feature`、`attitude_rate_complete_controller.feature`；同步检查 `4_Test/test_*.m` 契约测试与 `bus_contracts.m` 字段引用。
- **注意**：FlightCmdBus 废弃（CommandGenerator 旧输出）、NavigationContractBus v2 字段、NavigationStatusBus 制导状态流、MissionPlanBus A1 400 扁平化——所有引用这些字段的测试/脚本同步更新。
- **验收判据**：`.feature` 可被新字典解析、断言与 §4 字段一一对应；`test_bus_contracts.m` 批测全绿。
- **交付**：行为测试锁定新契约（此时模型仍旧，测试预期**红**——这是中间态，进入 M2 转绿）。
- **✅ 完成记录（2026-08-12）**：① `.feature` 全部移入 `4_Test/features/`（对齐本计划路径约定）；② 7 个 `.feature` 全面重写对齐 §4 新契约（`navigator`/`navigator_stop_and_go`/`commander_operational_state`/`mission_manager`/`mission_manager_structured_objective`/`mission_manager_land_completion`/`mission_manager_commander_integration`），`attitude_rate_complete_controller` 无需改；front-matter 换新字段路径、断言换新语义、删除引用已删行为的旧场景；③ L1 批测 `test_bus_contracts.m` **21/21 PASS**；④ `navigator`/`mission_manager` `.feature` 实测可被新字典解析、失败于模型编译（预期红，M2 转绿）；⑤ 旧字段残留扫描 0 命中。**范围排除（挂账 M2）**：`4_Test/test_*.m` 契约测试字段对齐（项目负责人指示 M1 忽略）。**模型接口挂账（M2）**：Commander 入改 `CommanderRequestBus` + 增 `CommandAckBus` 出；MissionManager 增 `MissionPlanAvailableBus` 出 + 触地判据驱动信号接口。

### M2 L3 模型字段对齐（主线程核心，G-004 遗留）
- **目标**：各 .slx 端口/信号/内部字段与 BusConfig 新 schema 对齐，模型可对新字典编译运行。
- **按数据流顺序改**（每模块改完跑对应行为测试回归）：
  1. **Navigator**：NavigationContract v2 字段消费 + NavigationStatusBus 制导状态流输出（`2_Model/navigator/`）。
  2. **Commander**：CommanderRequestBus 请求集（CommandId/Command/Params）+ 命令审查 + CommandAckBus 回报 + CommanderStatusBus（`2_Model/commander/`）。
  3. **CommandGenerator**：FlightCmdBus 废弃 → 移除或改 CommandAck 语义（旧统一命令接口，DEC-063）。
  4. **MissionManager**：MissionPlanBus A1 400 扁平化解析 + 运动学合同段推进 + RouteWindowBus 内部（`2_Model/mission_manager/`）。
  5. **Gateway**：MissionPlanBus A1 承载 + GcsLinkStatusBus 发布 + 三通道路由（`2_Model/command/` 相关 / `3_Integration/Gateway` 相关）。
  6. **FlightCore 集成**：新总线整体接线跑通（`3_Integration/FlightCore/`）。
  7. **传感器/plant**：GPSBus/IMUBus 驼峰改名同步（sensor_model 构建脚本已同步，核对 .slx 引用）。
- **验收判据**：`check_model_bus_usage.m`（L3）无字段悬空；各模型可对新字典编译；M1 行为测试由红转绿；V&V 前置的集成契约测试（`test_*_integration_contract.m`）通过。
- **交付**：模型层与新契约完全对齐，可运行参考架次雏形。

### M3 数值回填（控制分析，MBD——非访谈）
- **触发条件**：M2 完成后 + 13 §2.3 验证环境就绪。**未到条件不启动**（避免在坏模型上填数）。
- **动作**：按 §6 注册表逐项回填，来源 = 控制分析 / 仿真 / M400 benchmark（DEC-056）；同步按 DEC-122 触发 plant fidelity 增强（执行机构延迟/动态特性），增强记入 13 §2.3 fidelity 变化表。
- **纪律**：无来源不填数；填一个、回写关联文档（06/07/11/13）一个；仍无来源维持【暂定】。
- **验收判据**：控制分析结果填入 `product/16_Design_and_Control_Analysis.md`；VER-005/021/022 依赖数值就绪。

### M4 VER 执行（MIL → SIL）
- **顺序**：
  1. **VER-010 雏形最先跑**（R-02 前提：plant 能飞完参考架次五段），失败→触发 R-02 额外 plant 工作。
  2. VER-001 → 002..009（7 门）→ 011 / 014..021 逐条拆解与故障注入；VER-005/021/022 依赖 M3 数值。
  3. 每 VER：MIL 跑通 → SIL 跑通 → VCRM 结果列填 PASS/FAIL；关键项（VER-010/021）人工复核。
- **验证环境落地**：R-03 风险——VER-001 排程前先跑 MAVProxy FTP 上传雏形（真实 MAVLink FTP → Gateway → MM → MissionPlanAvailableBus），失败→切换 QGC 承担计划上传。
- **验收判据**：VER-001..022（后置项除外）在 VCRM 全 PASS；Validation 推理链（13 §7 链①②）闭合记录。
- **交付**：V&V 报告 + G-005 门禁过签 + 15 TBD 状态刷新。

---

## 6. 数值回填注册表（全部【暂定】/TBD，来源与触发）

> 完成即回写关联文档。回填主源 = 控制分析（DEC-056）；benchmark = M400 量级（DEC-013/051）。

| 数值 | 当前状态 | 回填来源 | 触发条件 | 关联 VER / 文档 |
|---|---|---|---|---|
| 控制周期 / 延迟 | TBD | 控制分析（DEC-056 七源） | M3 | VER-022（SIL 时序）/ 07 SYS-REQ-022 |
| 包线限值（速度/姿态/角速率/加速度硬限） | Vehicle Def【暂定】 | 控制分析 + M400 benchmark | M3 | VER-017/020/022 / 11 / 15 |
| 航点到达判据默认（位置 0.5 / 速度 0.3 / 高度 0.3 / 偏航 2°） | 【暂定】 | 控制分析（DEC-055） | M3 | VER-017 / 14 MM-010 |
| 到达判据连续周期数 N | TBD | 控制分析（DEC-095） | M3 | VER-017 |
| 段间跟随误差（MOP/停点位置 ≤0.1m） | TBD / 【暂定】 | 控制分析（DEC-018/056） | M3 | VER-017 / 06 |
| 能源裕度 / 整架次能源 | TBD | 控制分析 + 能源模型（需 plant 能源特征） | M3 + fidelity | VER-005 / 06 |
| CG 范围 | TBD | 载荷安装位置假设（Vehicle Def） | M3 | 11 |
| 触地判据阈值（六判据 + 时间保持 T） | TBD | 控制分析（DEC-041/100）+ fidelity（判据②旋翼卸载） | M3 + fidelity | VER-021 / 13 §6 |
| 抗风（飞行中 12 / 起降 8 m/s） | 【暂定】 | 控制分析（DEC-054） | M3 | VER-011/017 / 06 |
| 安全起飞高度 | 配置参数（值 TBD） | GCS 任务级配置（DEC-045） | 运行配置 | VER-014 |
| 单航线容量 400 / 单段 run ≤64 | 【暂定】 | 接口余量（DEC-083/085） | —（已定暂值） | VER-001 |
| 返航高度 H_rtl | TBD | 任务级配置（DEC-096） | M3 | VER-019 |
| RTK 精度（位置 ≤2~3cm / 速度 ≤0.1，固定） | 【暂定】 | SE 精度分析（DEC-068/105） | M3 | VER-003 / 14 SE-002 |
| 执行机构延迟（10~30 ms 基准） | 【暂定】 | M400 benchmark（DEC-051）→ 控制分析 | M3 + fidelity（DEC-122） | VER-022 / 11 |

**plant fidelity 触发（DEC-122，需要性驱动）**：数值回填需执行机构延迟与动态特性 → 增强动力模型到最小充分度；VER-021 判据②需非单调卸载信号 → 触地排程前确认动力 fidelity。每次增强记 13 §2.3 fidelity 变化表，受影响 VER 重跑。

---

## 7. 风险与前置约束（自包含摘要，完整版见 process/02）

| 风险 | 含义 | 缓解（已落地） |
|---|---|---|
| R-01 | 数值回填依赖（控制分析排程） | 注册表先建（§6），M3 前置条件明确 |
| R-02 | 现有 plant 可能无法飞完参考架次五段 | VER-010 雏形最先跑；失败→增强 plant（13 §2.3.1） |
| R-03 | MAVProxy FTP 子协议成熟度不足 | VER-001 前跑 FTP 雏形；失败→QGC 承担计划上传（13 §2.3.2） |
| R-04 | SE 链/文档漂移 | 文档树 + 门禁纪律（00 / process/00） |
| R-05 | 触地判据信号不可观测 | 13 §6 信号映射已成表 |
| R-06 | 首阶段范围膨胀 | §1.2 后置清单硬约束 |
| R-07 | 动力系统卸载信号能力不足（VER-021 判据②） | fidelity 触发机制（DEC-122）；VER-021 排程前确认 |

---

## 8. 过程纪律（执行时如何留痕）

- **决策**：新结论 → process/01（DEC 顺延 124+）+ 时间链「本次修订」块；修订须标原因/影响/是否重验。
- **风险 / 问题 / 门禁**：新风险 → process/02；VER 执行发现的问题 → process/03（**此时正式启用**）；里程碑过签 → process/04（G-005..）。
- **契约变更**：必须先改 4_Test 测试（约束 8），再改模型/消息/部署代码。
- **文档树 / 编号变更**：先登记 00 §3/§4，后使用。
- **代码生成注意**：generated FlightCore node 的异步订阅补丁在重新 codegen 后可能被覆盖，重生成后必须复核 `slros2_generic_pubsub.h` 与 `ros2nodeinterface.cpp`（README 约束 7）。

---

## 9. 完成判据（Definition of Done，第一阶段）

1. M2 模型层与新契约完全对齐，参考架次可编译运行（无 L3 字段悬空）。
2. M3 控制分析结果入册（product/16），关联文档数值同步、无凭空填数。
3. M4 VCRM 全 PASS（VER-012/013/014a 按后置标注 N/A），VER-010 全流程 + 关键项人工复核留证。
4. Validation 推理链（13 §7）闭合记录：第一阶段只闭合"执行前提"，不冒充实机确认。
5. G-005 门禁过签，15 TBD/Assumption 状态刷新，续接提示词（Prompt_Continue）同步。

---

*本文档为 Draft。建立依据 = 00 §3.2 过程链槽位（2026-08-12 登记）；执行过程中里程碑状态在此同步。*
