# UAV 单机飞控系统

> 自建无人机飞控实验基座。当前目标不是堆更多接口，而是在真实 AirSim -> WSL -> FlightCore 外部闭环中，跑出第一个带 manifest、可回放、可判据化的悬停 episode。

当前工程以 **四旋翼测试床** 为验收对象：AirSim 默认四旋翼 + `EscCmdBus.MotorCmd[4]`。目标产品仍是 30 kg 级六旋翼，但六旋翼参数和执行器布局尚未进入当前闭环。

## 当前主线

```text
AirSim endpoint (Windows)
  -> UDP JSON
  -> aircraft_udp_bridge (WSL2 / ROS2)
  -> flightcore_runtime_adapter
  -> /uav/* 六 topic
  -> FlightCore_ROS2_loop (Windows MATLAB, development exception)
  -> /uav/actuator/esc_cmd
  -> UDP actuator packet
  -> AirSim motor API
```

2026-07-07 架构审计后的裁决：

- 在第一个外部闭环 episode 通过前，不新增 `/uav/*` topic、观测族、仿真器适配器或命名迁移。
- `/uav/*` 六 topic 在开发期允许 DDS 跨 Windows <-> WSL2；这只是 Simulink 开发脚手架，不是终态部署规则。
- Windows <-> WSL2 的 simulator endpoint 路径仍然是 UDP-only。
- `/aircraft/*` 观测层只用于运行时适配、日志、评估和 evidence，不进入 FlightCore 控制闭环。
- 状态流水、完成度和会话交接只写入 PBOS；仓库文档只保留路线、契约、手册和执行计划。

详细执行顺序见 [DEVELOPMENT_PLAN.md](DEVELOPMENT_PLAN.md)。

## 架构边界

```text
┌─ Windows ───────────────────────────────────────────────┐
│ AirSim                                                   │
│   windows/airsim_udp_endpoint.py (AirSim endpoint)        │
│        │ UDP 56000: state / sensor_imu / sensor_gps      │
│        ▲ UDP 56001: actuator motor setpoint              │
├────────┼─────────────────────────────────────────────────┤
│ WSL2   ▼                                                 │
│ aircraft_udp_bridge                                      │
│        -> /aircraft/{state,imu,gps}                      │
│ flightcore_runtime_adapter                               │
│        -> /uav/sensors/imu                               │
│        -> /uav/sensors/gps                               │
│        -> /uav/cmd/flight                                │
│        <- /uav/actuator/esc_cmd                          │
│        <- /uav/estimator/state                           │
│        <- /uav/health/status                             │
├────────║  development DDS exception: /uav/* only          │
│ Windows▼                                                 │
│ MATLAB / Simulink                                        │
│   FlightCore_ROS2_loop                                   │
│     ROS2 message adapters                                │
│     FlightCore                                           │
│       IMU_BUS + GPS_BUS + FlightCmdBus                   │
│       -> EKF + UAV_FlightControl                         │
│       -> EscCmdBus + StateEstBus + health                │
└──────────────────────────────────────────────────────────┘
```

`FlightCore` 只消费和产出 InterfaceContract Bus。仿真器 API、ROS2/DDS 细节、AirSim 连接、WSL 节点生命周期、PlotJuggler、rosbag2、UDP schema 都不能进入 FlightCore 模型或产品数据字典。

## 文档体系

| 层级 | 权威文件 | 职责 |
|---|---|---|
| Tier 0 思想层 | [docs/vision/](docs/vision/) | 长期路线、消息哲学、主线约束，低频修改 |
| Tier 1 契约层 | [docs/contracts/](docs/contracts/) | FlightCore 接口边界、runtime isolation、DDS 例外 |
| 执行计划 | [DEVELOPMENT_PLAN.md](DEVELOPMENT_PLAN.md) | 当前最小行动序列、验收判据、冻结清单 |
| Tier 2 组件手册 | [bridge/airsim_ros2_udp_bridge/README.md](bridge/airsim_ros2_udp_bridge/README.md), [FC_SimulinkProject/3_Integration/ROS2/README.md](FC_SimulinkProject/3_Integration/ROS2/README.md) | 组件用法、同步规则、构建入口 |
| Tier 3 状态层 | PBOS `runtime/handoffs/UAVSingle.md` | 进度、会话交接、遗留事项，不在仓库内重复 |
| Archive | [docs/archive/](docs/archive/) | 审计报告、过期计划、legacy 脚本，只读参考 |

更完整的文档索引见 [docs/README.md](docs/README.md)。

## 仓库结构

```text
UAVSingleFlightControl/
├── README.md
├── DEVELOPMENT_PLAN.md
├── CLAUDE.md
├── AGENTS.md
├── docs/
│   ├── README.md
│   ├── vision/
│   ├── contracts/
│   └── archive/
├── FC_SimulinkProject/
│   ├── 1_Data_Dictionaries/
│   ├── 2_Model/
│   ├── 3_Integration/
│   ├── 4_Test/
│   └── 5_Tool/
└── bridge/
    └── airsim_ros2_udp_bridge/
```

WSL ROS2 workspace 是独立 git repo，不在 Windows 仓库中维护源码：

```text
~/uavsingle_ros2_ws/src/
├── aircraft_udp_bridge
├── flightcore_msgs
└── flightcore_runtime_adapter
```

Windows 仓库中的 `bridge/airsim_ros2_udp_bridge/` 只维护 Windows endpoint 与当前 endpoint 实际使用的 vendored schemas。UDP protocol、schemas 和 ROS2 bridge 的权威源在 WSL 原生 workspace。

## 强制约束

1. 所有子系统参数通过数据字典管理，不在模型块中硬编码数值。
2. Bus 定义只在 `FC_SimulinkProject/1_Data_Dictionaries/BusConfig/` 维护。
3. 契约变更必须先在 `FC_SimulinkProject/4_Test/` 加测试。
4. FlightCore 不得出现 AirSim、Gazebo、Isaac、ROS2、DDS、UDP、PlotJuggler、rosbag2 等运行时符号。
5. `truth`、`/aircraft/*` 和可视化数据不得作为 FlightCore 控制闭环输入。
6. actuator 主路径语义是 normalized motor setpoint，不退化为 throttle/roll/pitch/yaw。
7. Windows 侧文件在 Windows 仓库维护；WSL 侧源码在 WSL 原生 filesystem 维护；不要用 `/mnt/d` 作为长期 ROS2 workspace。
8. `build/`、`install/`、`log/`、`slprj/`、`derived/`、`__pycache__/`、episode 产物不纳入版本控制。

## 快速入口

MATLAB / Simulink：

```powershell
Start-Process -FilePath 'D:\MATLAB\R2025b\bin\matlab.exe' `
  -ArgumentList '-desktop' `
  -WorkingDirectory 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject'
```

打开工程：

```matlab
openProject('D:\Project\UAVSingleFlightControl\FC_SimulinkProject\FC_SimulinkProject.prj')
```

WSL ROS2 build：

```bash
cd ~/uavsingle_ros2_ws
source /opt/ros/jazzy/setup.bash
colcon build --merge-install --symlink-install
source install/setup.bash
```

Windows mock endpoint：

```powershell
python .\bridge\airsim_ros2_udp_bridge\windows\airsim_udp_endpoint.py `
  --bridge-host <WSL2_IP> `
  --mock `
  --duration-sec 10
```

常用操作规则见 [CLAUDE.md](CLAUDE.md)，Codex 差异见 [AGENTS.md](AGENTS.md)。
