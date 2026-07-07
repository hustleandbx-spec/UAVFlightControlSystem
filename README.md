# UAV Single Flight Control

30kg 级多旋翼无人机自研飞控系统 — 先建立可信的无人机行为实验基座，再让视觉、强化学习和世界模型在其中证明价值。

## 当前阶段主线

> **在真实的 AirSim↔WSL↔FlightCore 外部闭环中，跑出第一个"带 manifest 的可回放悬停 episode"——在此之前不新增任何接口、消息、观测族或改名。**

详细路线图：[docs/vision/UAV_World_Model_Capability_Roadmap.md](docs/vision/UAV_World_Model_Capability_Roadmap.md)

## 测试床 vs 产品机型声明

- **当前全部仿真/集成工作使用四旋翼测试床**（AirSim 默认四旋翼 + `EscCmdBus.MotorCmd[4]`）。控制律、混控器、contract 层均为四旋翼配置。
- **目标产品为 30kg 级六旋翼**，但六旋翼相关参数（六路混控矩阵、几何配置）尚未引入。当前阶段不应假设 4→6 的平滑升级不影响设计；混控器模块和 EscCmdBus 已预留为 MotorCmd 长度可扩展。

## 仓库结构

```
UAVSingleFlightControl/          ← git repo #1（Windows 端）
├── docs/
│   ├── vision/                  (Tier 0 思想文件，几乎不改)
│   ├── contracts/               (Tier 1 契约，先加测试再改)
│   └── archive/                 (过期文档，只读)
├── FC_SimulinkProject/          MATLAB/Simulink MBD 项目
├── bridge/
│   └── windows_endpoint/        仅保留 AirSim UDP endpoint + 4 份 vendored schema
└── 飞控开发需求.md               (待补充)
```

WSL ROS2 workspace 为独立 git repo #2（`~/uavsingle_ros2_ws/src`），仅维护三包：
- `aircraft_udp_bridge`
- `flightcore_msgs`
- `flightcore_runtime_adapter`

## 权威文档指针

| 内容 | 位置 |
|------|------|
| 路线图与阶段约束 | `docs/vision/UAV_World_Model_Capability_Roadmap.md` |
| 消息系统设计思想 | `docs/vision/自建飞控消息系统设计思想.md` |
| InterfaceContract（FlightCore 边界） | `docs/contracts/interface_contract.md` |
| Runtime Isolation（FlightCore 不依赖任何运行时） | `docs/contracts/flightcore_runtime_isolation.md` |
| ROS2 主题映射与 topics.yaml | `FC_SimulinkProject/3_Integration/ROS2/` |
| UDP JSON protocol + schemas（权威在 WSL） | `~/uavsingle_ros2_ws/src/aircraft_udp_bridge/` |
| Observation registry（evidence 资产，非运行时承诺） | WSL `flightcore_runtime_adapter/config/` + `docs/` |

## 项目强制约束

1. 所有子系统参数通过数据字典管理，不在模型块中硬编码数值
2. Bus 定义仅在 `1_Data_Dictionaries/BusConfig/` 中维护
3. 不将 `slprj/`、`derived/`、`build/`、`install/`、`log/` 纳入版本控制
4. 编写新工具函数前先检查 `5_Tool/` 下已有函数
5. FlightCore 不得出现仿真器/ROS2/DDS API 符号（有自动化检查护栏）
6. truth 不进入控制闭环；GPS fallback 默认关
7. 契约变更必须先在 `4_Test/` 加测试

## 相关资源

- 交接区（PBOS）：`handoffs/UAVSingle.md`、`projects/UAVSingle.md`
- Simulink MBD 技能：`.claude/skills/simulink-mbd.md`
- WSL workspace：`/home/hustle/uavsingle_ros2_ws`
