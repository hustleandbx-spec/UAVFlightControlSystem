# UAV 单机飞控系统

> 30kg 级多旋翼无人机自研飞控系统。主线是先建立可信的无人机行为实验基座，再让视觉、强化学习和世界模型在其中证明价值。  
> 当前全部工作基于 **四旋翼测试床**（AirSim 默认四旋翼 + `EscCmdBus.MotorCmd[4]`）；目标产品为 **30kg 级六旋翼**，六旋翼参数尚未引入。

---

## 一、当前阶段主线（2026-07-07 架构审计后冻结）

```
在真实的 AirSim → WSL → FlightCore 外部闭环中，跑出第一个
"带 manifest 的可回放悬停 episode"。

在此之前不新增任何接口、消息、观测族、改名或新战线。
```

详细路线图：[docs/vision/UAV_World_Model_Capability_Roadmap.md](docs/vision/UAV_World_Model_Capability_Roadmap.md)

---

## 二、架构审计核心诊断

2026-07-07 完成了全局架构审计（结合路线图、消息设计思想、契约文档、WSL 运行时实现），核心发现：

| # | 判断 | 依据 |
|---|------|------|
| 1 | **主线被稀释而非被替换**：V0 验收"可运行、可复现、可记录、可比较、可扩展"，但工程重心已滑到 V2/V4 层的接口预留上 | 13 个 WSL observation schema，但 bridge_node 只发 state 包；最近三次会话全是 observation layer/registry 工作 |
| 2 | **DDS 跨界存在未言明的矛盾**：UDP-only 规则与 Windows MATLAB 上 FlightCore 作为 ROS2 节点不能同时成立 | bridge README 禁止跨系统 DDS，但 ROS2 README 的主线路径是 Simulink ROS Toolbox |
| 3 | **抽象层过密**：同一个 IMU 穿过 7 层（AirSim → UDP JSON → sensor_msgs → mapping.py → flightcore_msgs → Simulink Sub → IMU_BUS） | 逐层比对 observation_protocol.md、bridge README、interface_contract.md |
| 4 | **第一性排序倒挂**：时间/序号/新鲜度是最弱的一环，却是飞控闭环最基础的需求 | IsNew 未使用、输出 sequence 常数 0、三个时钟域无定义 |
| 5 | **"可复现"地板缺失**：无版本控制、无 episode 格式定义、无 run manifest | git init == 0，全部文档搜不到 episode record 定义 |

**正面发现：** FlightCore 不依赖仿真器/ROS2/DDS API 的原则有自动化检查护栏（`RUNTIME_ADAPTER_ISOLATION_CONTRACT_PASS`），是项目最正确的架构决策。

---

## 三、当前架构总览

### 核心分层

```
┌─ Windows ──────────────────────────────────────────────┐
│  AirSim (endpoint = 唯一仿真器 API 域)                    │
│    airsim_udp_endpoint.py                                │
│      │ UDP 56000: state / sensor_imu / sensor_gps        │
│      ↑ UDP 56001: actuator packet                        │
├──────┼──────────────────────────────────────────────────┤
│ WSL2 ▼ (ROS2/DDS 主域)                                   │
│  aircraft_udp_bridge ─→ /aircraft/{state,imu,gps}        │
│         │                        │                       │
│  flightcore_runtime_adapter      ├─→ rosbag2 + manifest  │
│         │                        │   (episode 存储)       │
│         ▼                        └─→ PlotJuggler         │
│  /uav/* 六 topic（冻结契约）                               │
├──────┼──────────────────────────────────────────────────┤
│      ║ 【已注册的开发期例外】跨界 DDS best-effort          │
│      ║ 仅限 /uav/* 六 topic，随 codegen 入 WSL 自动废止   │
├──────▼ Windows MATLAB ────────────────────────────────┤
│  FlightCore_ROS2_loop:                                  │
│   ROS2Sub → ROS2ToFlightCoreBus → FlightCore → …ToROS2 │
│              (IMU_BUS/GPS_BUS/FlightCmdBus → EscCmdBus) │
└─────────────────────────────────────────────────────────┘
```

### FlightCore 内部结构

FlightCore.slx 是飞控产品核心，仅消费和产出 InterfaceContract Bus：

```
输入边界：IMU_BUS + GPS_BUS + FlightCmdBus
  → EKF（当前集成基线，ESKF/UKF 保留为研究资产）
  → UAV_FlightControl（串级控制链）
    → 位置 PID → 速度 PID → 加速度转姿态
    → 姿态 P → 角速率 PID → 混控器
输出边界：EscCmdBus + StateEstBus + 遥测
```

### 数据字典体系

```
VehicleDict (mass, inertia, g, d, rotor_count)
    ↑ 引用
    ├── DynamicModelDict
    ├── PowerSystemDict   (U_bat, Cb, Ct, Cm, w_bias, Jrp)
    └── FlightControlDict (PID增益 + 限幅 + mixMatrix)

StateEstDict (g_n, dt_imu, sigma_*, R_*, P0_*, ukf_*)
    ↑ 引用
    ├── GlobalTypes (IMU_BUS, GPS_BUS, StateEstBus, ...)
    └── VehicleDict
```

所有参数通过数据字典管理，禁止在模型块中硬编码数值。

### DDS 跨界例外条款

详见 `docs/contracts/flightcore_runtime_isolation.md`。

**例外范围：** 开发期 `/uav/*` 六 topic 允许 DDS 跨 Windows↔WSL2（禁用此路径上任何 `/aircraft/*` 的 DDS）。
**废止条件（全部满足后自动生效）：**
1. 外部闭环 episode 已验证通过
2. FlightCore 控制/估计逻辑进入低频修改期
3. M6 代码生成里程碑正式启动

---

## 四、技术栈

| 层 | 技术 | 阶段 |
|----|------|------|
| 建模与仿真 | MATLAB R2025a+ / Simulink / Stateflow | **当前** |
| 数据管理 | Simulink Data Dictionary (.sldd) | **当前** |
| 运行时集成 | WSL2 Ubuntu 24.04 + ROS2 Jazzy + Python | **当前** |
| 外部仿真器 | AirSim（已被微软归档，验证中） | **当前** |
| 可视化/日志 | PlotJuggler + rosbag2（WSL 侧） | **当前** |
| 消息生成 | flightcore_msgs / topics.yaml 单一事实源 | **当前** |
| C++ 中间件 | FlightBus 命名空间（三缓冲无锁发布/订阅） | **已推迟**（superpower 时期设计） |
| 测试 | Simulink Test + MATLAB Unit Test | 框架已有，覆盖率 ~30% |

---

## 五、仓库结构

```
UAVSingleFlightControl/              ← git repo #1（Windows 端）
├── docs/
│   ├── vision/                      路线图、消息设计思想（几乎不改）
│   ├── contracts/                   InterfaceContract、Runtime Isolation（含 DDS 例外）
│   └── archive/                     过期计划、legacy 脚本（只读）
├── FC_SimulinkProject/              MATLAB/Simulink MBD 项目（.prj）
│   ├── 1_Data_Dictionaries/         数据字典 + BusConfig
│   ├── 2_Model/                     FlightCore、ESKF/EKF/UKF、动力学模型
│   ├── 3_Integration/               ROS2 映射 + FlightCore_ROS2_loop
│   ├── 4_Test/                      测试框架
│   └── 5_Tool/                      工具函数
├── bridge/
│   └── airsim_ros2_udp_bridge/      Windows 侧 AirSim endpoint + 4 份 vendored schema
└── .gitignore
```

WSL ROS2 workspace（独立 git repo #2，`~/uavsingle_ros2_ws/src`）：
- `aircraft_udp_bridge` — UDP ↔ ROS2 桥接
- `flightcore_msgs` — 自定义消息定义（与 Windows `3_Integration/ROS2/flightcore_msgs/` 同步）
- `flightcore_runtime_adapter` — `/aircraft/*` → `/uav/*` 语义转换

---

## 六、权威文档指针

| 内容 | 位置 |
|------|------|
| 路线图与阶段约束 | `docs/vision/UAV_World_Model_Capability_Roadmap.md` |
| 消息系统设计思想 | `docs/vision/自建飞控消息系统设计思想.md` |
| InterfaceContract（FlightCore Bus 边界） | `docs/contracts/interface_contract.md` |
| Runtime Isolation 契约（含 DDS 例外条款） | `docs/contracts/flightcore_runtime_isolation.md` |
| ROS2 topic 映射 + topics.yaml | `FC_SimulinkProject/3_Integration/ROS2/` |
| UDP JSON protocol + schemas 权威 | WSL `~/uavsingle_ros2_ws/src/aircraft_udp_bridge/` |
| Observation registry（evidence 资产） | WSL `flightcore_runtime_adapter/config/` |
| Simulink MBD 操作指引 | `CLAUDE.md` |
| 项目状态与交接 | PBOS `handoffs/UAVSingle.md` |

---

## 七、项目强制约束

1. 所有子系统参数通过数据字典管理，不在模型块中硬编码数值
2. Bus 定义仅在 `1_Data_Dictionaries/BusConfig/` 中维护
3. 不将 `slprj/`、`derived/`、`build/`、`install/`、`log/` 纳入版本控制
4. 编写新工具函数前先检查 `5_Tool/` 下已有函数
5. **FlightCore 不得出现仿真器/ROS2/DDS API 符号**（有自动化检查护栏 `RUNTIME_ADAPTER_ISOLATION_CONTRACT_PASS`）
6. **truth 不进入控制闭环**；`gps_fallback_from_state` 默认关闭
7. **契约变更必须先在 `4_Test/` 加测试**
8. `/aircraft/*` 观测数据只能用于监控、日志、评估，不是 FlightCore 传感器输入
9. actuator 主路径语义 = normalized motor setpoint（不是 throttle/RPY，不是 PWM/DShot/RPM）

---

## 八、下一步行动（优先级排序）

| # | 行动 | 判据 |
|---|------|------|
| 🔴 1 | Mock 端到端 smoke：`--mock` endpoint → WSL 两节点 → MATLAB 收 `/uav/sensors/imu`、回 `/uav/actuator/esc_cmd` | mock 日志出现 actuator packet；三时钟域偏差实测落盘 |
| 🔴 2 | AirSim API 探针：`moveByMotorPWMsAsync` 支持性 + 四电机响应 + 旋转方向 | API 存在，`--motor-order` 定值 |
| 🔴 3 | 真实 AirSim 悬停 30s 外部闭环 + rosbag2 + manifest.yaml + 判据脚本 | 目录可整体拷走回放；truth-vs-estimate 出图；判据 PASS/FAIL |
| 🟡 附 | 附着在 #3 内：sequence 递增、health/status 发布、IsNew 策略定值 | — |

### 当前冻结清单

| 冻结项 | 解冻条件 |
|--------|----------|
| barometer/magnetometer/rangefinder/lidar 运行时落地 | 控制需求拉动（如 EKF 需要气压高度融合） |
| /aircraft/* naming 迁移（/aircraft/state → truth/state） | 外部闭环验证通过后 |
| EscCmd/SystemHealth 改名 | 闭环跑通前不动 |
| Gazebo/Isaac/Pegasus adapter | 各自的里程碑启动 |
| MAVLink Gateway | M5 故障安全实现启动 |
| 任何新增 /uav topic | 六 topic 冻结至 V0 验证通过 |
| 任何 RL/视觉/world-model 接口实现 | 路线图 V3 阶段 |
| C++ FlightBus 中间件 | 路线图 V4 阶段 |
