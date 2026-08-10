# FlightCore — 09 System Logical Architecture（系统逻辑架构）

## 文档信息

| 项目 | 内容 |
|---|---|
| 文档名称 | FlightCore System Logical Architecture（系统逻辑架构） |
| 版本 | v0.1 |
| 状态 | **Draft（逻辑架构 v1 + 物理分配，2026-08-06）** |
| 来源 | `process/01_Session_and_Decision_Log.md` DEC-072/074/075/087 |
| 上游文档 | 08_System_Functional_Architecture；10_Interface_Definition |
| 维护规则 | 数值类（周期/延迟/频率/负载）留 DEC-056 控制分析产出，不在此填数 |

## 1. 逻辑架构 v1（DEC-074）

### 1.1 模块集

Gateway / Commander / MissionManager / Navigator（制导）/ FlightControl / StateEstimation / Logging + FailSafe（处置挂起 TBD-014）/ 参数系统（后置 TBD-015）。

### 1.2 速率分层

| 层 | 内容 |
|---|---|
| 高速（控制率） | FlightControl（含混合器）、制导求值、StateEstimation IMU 融合 |
| 中速 | RTK 气压融合、MissionManager 段推进判据、Commander 状态视图 |
| 低速事件 | Commander 命令、Gateway 遥测、Logging 落盘 |

数值（周期/延迟/频率）留 DEC-056 控制分析。

### 1.3 接口机制（DEC-072）

判据 = **时序要求**，非消费者数量：强时序（确定周期/低延迟/有界/可验证）→ **直连端口总线**（控制环链路：SE→FC 估计状态、制导→FC 轨迹参考、FC→电机指令）；无严格时序要求 → **订阅发布**（运动学合同、状态回报、命令事件、航线数据、遥测、日志）。

### 1.4 依赖关系

航线→MM；指令→Commander；合同→Navigator；参考→FC；状态回报→Commander/Gateway/Logging。

## 2. 模块状态流体系（DEC-087）

除 Commander 外，每个模块对其状态/标志位的对外发布 = **每条模块一条"状态流"**（订阅发布，宽松时序）——模块为自身状态权威生产者、一次发布、多消费者订阅组合；**不按消费者拆流**。消费者 = Commander（订阅全部模块状态流 → 状态视图/命令审查，读可行性标志位）+ Logging（订阅落盘）+ Gateway（订阅转发 GCS 遥测）。**控制环直连总线（StateEstBus→FC、TrajectorySetpointBus→FC、EscCmdBus→电机）不并入状态流体系**；任何控制/判据不得依赖状态流。

- 制导状态流 = NavigationStatusBus（承载完整状态含 Valid，DEC-087）
- SE 状态流 = StateEstBus 数据经订阅发布（Commander 从订阅读 Status；控制环仍走直连）

## 3. 物理分配（DEC-075，Assumption 层）

- **名义部署目标 = 飞控 MCU（Edgi-X/PSOC E83 级：400MHz M55 双核、5MB SRAM、16MB PSRAM、16MB XIP FLASH）+ 伴机（未来两层架构）**；实机选型后置（DEC-033 第一阶段纯仿真）。
- **不因视觉导航/高级算法试验计划上调 MCU 规格（A-007）**：视觉导航在伴机，FC 只收有界外部参考。
- **负载预算框架 = SYS-GOAL-05 峰值 ≤50~60% 带宽余量**；伴机接口预算（~1~5%，Assumption）显式列为 DEC-056 实时性分析项；数值由 **MIL/SIL profiling 实测**，不拍脑袋。
- **扩展双路径** = 机载余量内 / 伴机（A-007），FlightCore 接口不变。

## 4. 规则十二 = 数据解耦（DEC-064）

第一阶段四旋翼不设独立 Mixer 模块（固定 4×4 常矩阵，独立模块收益≈0）；混合器作为 FlightControl 内部子功能。**数据解耦而非模块拆解**：①混合器矩阵参数来自 Vehicle Definition（构型几何：轴距/臂长/电机布局），不硬编码；②FlightControl 内部按"虚拟控制量 {F, Mx, My, Mz} → 分配 → 电机指令"边界组织，为未来复合翼/倾转旋翼留**提取点**。

## 5. 备注

- **DEC-067**：五阶段 = 人为划分的过程视图，非实现状态机（见 08 §5）。
- **DEC-073**：航段内轨迹为预规划固定曲线，制导每控制周期按时间求参考值 r(t_k)={p, v, yaw}——纯时间函数、确定性；估计状态由 StateEstimation 提供；FC 闭环误差 = 参考 − 估计状态。StateEstimation→Navigator 仅段边界衔接用（DEC-065 细化）。
