# FlightCore — 12 Parameter and Configuration Management（参数与配置管理）

## 文档信息

| 项目 | 内容 |
|---|---|
| 文档名称 | FlightCore Parameter and Configuration Management（参数与配置管理） |
| 版本 | v0.1 |
| 状态 | **Draft（占位，2026-08-08 建立）** |
| 来源 | 会话·决策日志 TBD-015；CLAUDE.md 约束 2/4；10_Interface_Definition（config 层） |
| 上游文档 | 10_Interface_Definition；11_Vehicle_Definition |
| 维护规则 | SE 层定义参数/配置边界；运行时参数系统实现后置 |

## 1. 参数/配置边界（SE 层）

| 类别 | 归属 | 载体 |
|---|---|---|
| 车辆假设基准 | Vehicle Definition | 11 |
| 任务级配置 + 航点级覆盖 | 任务计划 schema | 10（A1 config/waypoint） |
| 数据字典 / Bus 定义 | 建模参数 | `FC_SimulinkProject/1_Data_Dictionaries/` |
| 运行时参数系统 | **后置（TBD-015）** | — |

## 2. 运行时参数系统（后置占位）

- 需求：GCS 查询面 + FC 运行时参数读写（PX4 风格）。
- 状态：**后置（TBD-015）**；重启触发条件待补。
- 衍生待议：运行时改参数与当前飞行状态的关系。

## 3. 配置管理（文档层）

- git + DEC 日志 + 文档状态生命周期。
- 详见 00_SE_Management_Plan §5。
