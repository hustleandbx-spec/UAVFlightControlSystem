# 文档体系索引

> 本目录按 2026-07-07 架构审计后的治理规则组织：思想、契约、组件手册、状态分离。仓库内不维护会话流水和完成百分比。

## 分层

| 层级 | 目录/文件 | 更新频率 | 内容边界 |
|---|---|---:|---|
| Tier 0 思想层 | `docs/vision/` | 低 | 为什么做、长期路线、消息架构哲学 |
| Tier 1 契约层 | `docs/contracts/` | 中 | FlightCore 接口、runtime isolation、DDS 例外、边界不变量 |
| 执行计划 | `../DEVELOPMENT_PLAN.md` | 中 | 当前阶段行动序列、验收判据、冻结/解冻规则 |
| Tier 2 组件手册 | 组件目录内 `README.md` / `protocol.md` | 中 | 具体组件如何运行、构建、同步 |
| Tier 3 状态层 | PBOS `runtime/handoffs/UAVSingle.md` | 高 | 会话交接、完成状态、遗留事项 |
| Archive | `docs/archive/` | 只读 | 审计报告、过期计划、legacy 脚本 |

## 权威文件

| 问题 | 读这里 |
|---|---|
| 当前项目主线是什么 | `../README.md` |
| 下一步怎么做、怎么验收 | `../DEVELOPMENT_PLAN.md` |
| 长期路线和克制原则是什么 | `vision/UAV_World_Model_Capability_Roadmap.md` |
| 为什么内部事实层不能被 ROS2/MAVLink 定义 | `vision/自建飞控消息系统设计思想.md` |
| FlightCore Bus 边界是什么 | `contracts/interface_contract.md` |
| RuntimeAdapter 和 DDS 跨界例外怎么定义 | `contracts/flightcore_runtime_isolation.md` |
| Windows endpoint 怎么跑 | `../bridge/airsim_ros2_udp_bridge/README.md` |
| UDP JSON packet 怎么定义 | `../bridge/airsim_ros2_udp_bridge/protocol.md` |
| ROS2 topic 与 Simulink Bus 怎么映射 | `../FC_SimulinkProject/3_Integration/ROS2/README.md` |
| ESKF 研究模型为什么不在当前主线 | `../FC_SimulinkProject/2_Model/state_estimation/ESKF/KNOWN_ISSUES.md` |
| Claude/Codex 怎么启动 MATLAB/MCP | `../CLAUDE.md` / `../AGENTS.md` |
| 最近做到了哪里 | PBOS `runtime/handoffs/UAVSingle.md` |

## 文档写入规则

1. 新增长期思想或路线判断，放入 `docs/vision/`。
2. 新增接口、不变量、跨系统边界，放入 `docs/contracts/`。
3. 新增具体组件用法，放入组件目录的 README 或 protocol 文件。
4. 新增短期行动序列和验收判据，更新 `DEVELOPMENT_PLAN.md`。
5. 新增会话状态、进度、遗留项，写入 PBOS handoff，不写入仓库。
6. 过期但有历史价值的计划放入 `docs/archive/`，并在文件头声明已归档。

## 当前冻结面

外部闭环 episode 通过前，下列内容不得通过文档暗示为当前主线：

- Gazebo、Isaac、Pegasus runtime adapter
- MAVLink Gateway
- 新增 `/uav/*` topic
- EscCmd/SystemHealth 改名
- `/aircraft/*` naming 迁移
- barometer、magnetometer、rangefinder、lidar 运行时落地
- RL、视觉、world model 接口实现
- C++ FlightBus 中间件

这些内容可以作为 vision 或 archive 中的历史/未来方向存在，但不能出现在 README 或开发计划的当前执行路径中。
