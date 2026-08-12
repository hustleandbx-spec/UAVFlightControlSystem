# UAV 单机飞控系统

> 面向工业无人机的自主 FlightCore 飞控系统，基于 MATLAB/Simulink 模型化设计（MBD）与 SE 文档体系建设的个人长期工程项目。

## 项目背景

本项目旨在研制一套面向工业无人机应用、具备真实工程落地可能的自主 FlightCore 系统，不以复刻现有开源飞控或单纯验证某种控制算法为目标，而是建立一条**完整、可追溯、可验证的飞控工程链**：

```text
需求定义 → 系统架构 → 飞行动力学建模 → 状态估计 → 制导与控制
  → 控制分配 → 执行机构 → 软件实现 → MIL → SIL → HIL → 实机试飞 → 需求验证与迭代
```

- **参考机型**：RV-Q01 大型工业四旋翼（DJI Matrice 400 量级），当前处于 Assumption 层，正式参数基线待 Vehicle Definition 回填。
- **目标应用**：工业巡检 / 观测侦察，户外、GNSS/RTK 可用环境。
- **工程方法**：按真实飞控产品的工程逻辑建设——需求可追溯、模型化设计、自动化验证、参数经数据字典管理、代码生成以及 MIL/SIL/HIL 连续验证。
- **当前主线（第一阶段，2026-08-12 起）**：MIL + SIL 两级纯仿真，让 FlightCore 正常流全流程（起飞 → 出航 → 任务 → 返航 → 降落 → 触地）按要求跑通并完成 V&V（VER-001..022）。涉及 7 个模块：Gateway / Commander / MissionManager / Navigator / FlightControl / StateEstimation / Logging。
- **架构原则**：FlightCore 与具体飞行器解耦；Mission / Guidance / Safety / Control 职责分离；GCS 负责任务规划与语义校验，FlightCore 为场景无关的执行者。

> 文档体系入口（总索引）：[`docs/README.md`](docs/README.md)。
> SE 体系组织：见总索引内 [`docs/00_SE_Management_Plan.md`](docs/00_SE_Management_Plan.md)。
> 当前开发执行（M1..M4）：见总索引内 [`docs/process/05_Development_Plan.md`](docs/process/05_Development_Plan.md)。

## 文件索引

### 仓库顶层

| 路径 | 内容 |
|---|---|
| `README.md` | 本文件：项目背景 + 文件索引 |
| `CLAUDE.md` | Claude Code 操作指引（启动 / MCP / 参数流程 / 项目约束） |
| `AGENTS.md` | Codex 差异配置 |
| `FC_SimulinkProject/` | MATLAB/Simulink 工程（模型 / 数据字典 / 测试 / 工具） |
| `docs/` | SE 文档体系（宪法层 + 产品链 + 过程链） |
| `bridge/` | Windows–WSL 桥接（历史资产，非当前主线） |
| `orchestration/` | Windows 侧运行编排脚本（历史资产） |

### docs/ — SE 文档体系

| 路径 | 内容 |
|---|---|
| `00_SE_Management_Plan.md` | SE 体系宪法层：文档树 / 编号 / 配置管理 / 门禁声明 |
| `README.md` | 文档体系索引 |
| `product/` | 产品链 01..16：愿景 / 需要 / ConOps / 需求 / 架构 / 接口 / 车辆定义 / V&V / 子系统需求 / 设计分析 |
| `process/` | 过程链 00..05：管理 / 决策日志 / 风险 / 问题 / 门禁 / **开发计划（实施入口）** |
| `vision/` | 长期路线图与消息系统设计思想 |
| `archive/` | 只读归档：审计报告、过期计划、legacy 脚本与早期契约/设计 |
| `Prompt.txt` / `Prompt_Continue.md` | 会话续接提示词 |

### FC_SimulinkProject/ — MATLAB/Simulink 工程

| 路径 | 内容 |
|---|---|
| `FC_SimulinkProject.prj` / `start.m` / `shutdown.m` | 工程入口与启停脚本 |
| `1_Data_Dictionaries/` | 数据字典与权威参数源 |
| `└─ BusConfig/` | 21 条 Bus 定义（`config_*.m`，含 A2 契约 #1..7） |
| `└─ ParamSources/` | 参数 YAML 权威源（各子系统 `*_params.yaml`） |
| `└─ *.sldd` + `create_*.m` | Simulink 数据字典与重建脚本 |
| `2_Model/` | 子系统模型 |
| `└─ control/` `command/` | 控制律、命令生成 |
| `└─ navigator/` `commander/` `mission_manager/` | 制导 / 命令门卫 / 任务管理（新契约模块） |
| `└─ state_estimation/` | 状态估计（EKF / ESKF / UKF） |
| `└─ sensor_model/` `dynamic_model/` `power_system/` | 传感器 / 动力学 / 动力系统模型 |
| `3_Integration/` | 集成层 |
| `└─ FlightCore/` `FlightCore_Gazebo_loop.slx` `FlightCore_ROS2_loop.slx` | FlightCore 与联合仿真 harness |
| `└─ MAVLink/` | MAVLink Gateway 模型 |
| `└─ Gazebo/` `ROS2/` | 联合仿真 / ROS 2 接口（历史资产） |
| `4_Test/` | 测试：`features/*.feature` 行为测试、`test_*.m` 契约测试、`bus_contracts/`、`check_model_bus_usage.m` |
| `5_Tool/` | 工具函数（字典重建、四元数、YAML 读取等） |
| `third_party/` | 第三方库（yaml 解析等） |

> 联合仿真（Gazebo 锁步 / AirSim / ROS 2 standalone node）与 `bridge/`、`orchestration/` 均已退出当前主线，作为历史资产保留。当前唯一开发主线见 [`docs/process/05_Development_Plan.md`](docs/process/05_Development_Plan.md)。
