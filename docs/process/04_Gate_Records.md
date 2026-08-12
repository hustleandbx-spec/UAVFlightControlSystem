# process/04 — 评审门禁过签记录（Gate Records）

## 文档信息

| 项目 | 内容 |
|---|---|
| 文档名称 | 评审门禁过签记录 |
| 版本 | v0.1 |
| 状态 | **Draft（2026-08-10 建立；G-001..004 已记）** |
| 角色 | 记录 SE 层门禁的过签证据（谁评审、依据、遗留项）；门禁清单 = `01_Session_and_Decision_Log.md` §12 SE 链矩阵 |
| 上位文档 | `00_SE_Management_Plan.md`（门禁声明）；`01_Session_and_Decision_Log.md` §12（DEC-080 门禁清单） |
| 维护规则 | 每层 Draft → Reviewed → Baselined 时记一行；遗留项显式标注 |

## 1. 门禁过签记录

| G-ID | SE 层 / 文档 | 评审依据 | 过签日期 | 遗留项 | 状态 |
|---|---|---|---|---|---|
| G-001 | SYS-REQ-001..022（product/07） | 01 §12 SE 链矩阵；DEC-110/121 | 2026-08-10 | 数值类需求待控制分析回填（属 MOP 层，非需求缺口） | Reviewed |
| G-002 | SUBSYS-REQ（product/14） | 01 §12 第③缺口；A3 守则（DEC-091/111） | 2026-08-10 | 无 | Reviewed |
| G-003 | SE 链门禁第⑤项（SYS-REQ-010/011） | DEC-110/121 | 2026-08-10 | 无 | 闭合 |
| G-004 | A2 接口契约层（product/10 §4 + bus_contracts.m + GlobalTypes.sldd 21 总线） | 01 §12 接口·计划 schema / 接口·模块契约；bus_contracts 批测 21/21 PASS（L1）；create_GlobalTypes 重建（L2）；check_model_bus_usage 覆盖 | 2026-08-12 | L3 模型字段对齐（Navigator/Commander/CommandGenerator 引用旧字段，编译即失败）= 实现层待办，模型重构时核 | Reviewed |

## 2. 说明

- 门禁清单与各层状态权威 = 01 §12 SE 链矩阵（DEC-080：进入第一阶段开发前逐层走完、无空白）。
- 文档状态生命周期 = Draft → Reviewed → Baselined（00 §3）；**不擅自 Baseline**，须经项目负责人确认。
- 本表只记"过签证据"，不重复门禁内容。
