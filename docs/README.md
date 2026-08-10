# FlightCore 文档体系索引

> 按 **2026-08-10 SE 体系重构**：`docs/` 组织为「宪法层 + 产品链 + 过程链」。SE 体系入口 = `00_SE_Management_Plan.md`。

## SE 体系总览

| 链 | 位置 | 回答 | 说明 |
|---|---|---|---|
| 宪法层 | `00_SE_Management_Plan.md` | 体系怎么组织 | 文档树 / 标准映射 / 术语编号 / 配置管理 / 门禁声明 |
| 产品与技术链 | `product/` | 做什么、做成什么样、怎么验证 | 01..15（需求 / 设计 / 接口 / V&V） |
| 过程管理链 | `process/` | 怎么定、怎么管、出过什么问题、门过了没有 | 00..04（决策 / 风险 / 变更 / 问题 / 评审） |
| 合规/适航轨 | 显式后置 | 法规合规 | TBD-002（后置，非 SE 文档） |

## 产品与技术链（product/）

| 槽位 | 文件 | 内容 |
|---|---|---|
| 01 | `product/FlightCore_Project_Vision_v0.1_Draft.md` | 项目愿景 |
| 02 | `product/02_Stakeholders_and_Needs.md` | 利益相关方与需要（占位待回填） |
| 03 | `product/03_ConOps.md` | 运行概念 |
| 04 | `product/04_Reference_Missions.md` | 参考架次 |
| 05 | `product/05_ODD_and_Environment.md` | 运行设计域与环境 |
| 06 | `product/06_MOE_MOP_TPM.md` | 效能 / 性能 / 技术性能度量 |
| 07 | `product/07_System_Requirements.md` | 系统需求（SYS-REQ） |
| 08 | `product/08_System_Functional_Architecture.md` | 功能架构 |
| 09 | `product/09_System_Logical_Architecture.md` | 逻辑架构 |
| 10 | `product/10_Interface_Definition.md` | 接口定义（A1 任务计划接口 + A2 契约 #1..7） |
| 11 | `product/11_Vehicle_Definition.md` | 车辆定义（Assumption 层） |
| 12 | `product/12_Parameter_and_Configuration_Management.md` | 参数与配置（占位） |
| 13 | `product/13_V&V_Plan.md` | 验证与确认计划 |
| 14 | `product/14_SUBSYS_REQ.md` | 子系统需求 |
| 15 | `product/15_Assumptions_Constraints_TBD.md` | 假设 / 约束 / TBD |

## 过程管理链（process/）

| 文件 | 角色 |
|---|---|
| `process/00_Process_Management.md` | 过程管理说明（边界 / 生命周期 / 变更四问） |
| `process/01_Session_and_Decision_Log.md` | 会话·决策日志（修订时间链 + DEC 表 + 产品原料历史溯源） |
| `process/02_Risk_Register.md` | 风险登记册（R-xx） |
| `process/03_Issue_Register.md` | 问题 / 缺陷登记（I-xx） |
| `process/04_Gate_Records.md` | 评审门禁过签记录（G-xx） |

## 权威文件

| 问题 | 读这里 |
|---|---|
| SE 体系怎么组织 / 从哪进 | `00_SE_Management_Plan.md` |
| 决策记录 / 时间链 / DEC | `process/01_Session_and_Decision_Log.md` |
| 当前项目主线是什么 | `../README.md` |
| 下一步怎么做、怎么验收 | `../DEVELOPMENT_PLAN.md` |
| 最近做到了哪里 | PBOS `runtime/handoffs/UAVSingle.md` |
| 怎么启动 MATLAB/MCP | `../CLAUDE.md` / `../AGENTS.md` |

## 辅助目录（历史 / 探索，非 SE 文档树）

| 目录 | 内容 |
|---|---|
| `vision/` | 长期路线与消息思想（CLAUDE.md 架构指针引用） |
| `archive/` | 只读归档：审计报告、过期计划、legacy 脚本 + 早期 `contracts/` `design/` `specs/` `reports/` `architecture_discussions/` |

## 文档写入规则

1. SE 体系内容按链写入：产品内容 → `product/`，过程内容 → `process/`。
2. 文档树 / 编号 / 术语变化先在此登记：`00_SE_Management_Plan.md` §3/§4（**先登记后使用**）。
3. 新决策 → 会话·决策日志（DEC 顺延 + 时间链块）。
4. 新风险 → `process/02`；新问题 → `process/03`；门禁过签 → `process/04`。
5. 过期但有历史价值 → `archive/`，文件头声明已归档。
6. 会话状态 / 进度 / 遗留项 → PBOS handoff，不写入仓库。
