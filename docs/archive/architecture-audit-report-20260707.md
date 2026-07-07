# UAV 单机飞控系统架构审计报告

> 日期：2026-07-07  
> 范围：UAVSingleFlightControl 全项目（Simulink 模型 + WSL ROS2 workspace + 文档体系）  
> 方法：第一性原理 + 奥卡姆剃刀  
> 前置阅读：路线图、消息设计思想、InterfaceContract、Runtime Isolation、WSL 运行时、observation registry

---

## 一、核心张力（先于结论）

项目同时存在三份"宪法"，各自内部自洽，但相互之间的优先级已经漂移：

1. **路线图宪法**（`UAV_World_Model_Capability_Roadmap.md` §9-§12）：第一阶段只追求"可运行、可复现、可记录、可比较、可扩展"，V0 = 最小飞控闭环 + 日志 + 指标 + 失败样本。
2. **消息宪法**（`自建飞控消息系统设计思想.md` §2、§9）：内部事实层为中心的辐射结构，ROS2 只是投影，Gateway 防止外部协议污染内核。
3. **运行时宪法**（`flightcore_runtime_isolation.md` + WSL `observation_bridge_README.md`）：FlightCore/InterfaceContract/RuntimeAdapter 三件套，外加 `/aircraft/*` 观测层、13 个 packet schema、4 仿真器 evidence registry。

张力在于：宪法 1 说 V0 的验收是"悬停 + 日志 + 失败复现"，但工程投入的重心已经滑到宪法 3 的"数据如何流动"上，而宪法 1 真正要的"数据如何被记录、比较、复现"至今为零。同时宪法 2 的"UDP-only 边界"与当前 Simulink 主线存在一个没人明说的矛盾。

---

## 二、核心判断（5 条）

### 判断 1：主线没有被"替换"，但已经被"稀释"

V0 验收项被降级为最低优先级，而 V2/V4 层的预留工作占据了最近全部工程量。

**依据：** `UAVSingle.md` 下一步清单中，"短时悬停自动判据"排在 🟢 第 6 位，而前 5 位全是 ROS2/UDP/endpoint 联调；`handoffs/UAVSingle.md` 最近三次会话记录全部是 observation layer / workspace 清理 / registry 工作；路线图 §3 的自检问题"它是在增强我的实验基座，还是在制造新战线？"对 observation registry 的 13 观测族，答案是后者——WSL 有 13 个 schema，但 `bridge_node.py` 实测只处理 `state` packet（`validate_state_packet` 是唯一入口）。**协议宪法领先运行实现两个身位，这正是路线图 §11 警告的"闭环冒进"的镜像形式：接口冒进。**

### 判断 2：存在一个未被言明的架构矛盾——"UDP-only"与"FlightCore 在 Windows MATLAB 上作为 ROS2 节点运行"不能同时成立

**依据：** `bridge/airsim_ros2_udp_bridge/README.md` Boundary Rules 明确写 "Do not enable ROS2/DDS discovery across Windows/WSL2" 和 "Do not run FlightCore ROS2 nodes on Windows for this path"；但 `3_Integration/ROS2/README.md` 的主线路径是 Simulink ROS Toolbox Subscribe/Publish，且已验证 "WSL2 to MATLAB … MATLAB `ros2subscriber`" ——这就是 DDS 跨 Windows/WSL2。当前事实拓扑是：`Windows AirSim →UDP→ WSL2 →DDS 跨界→ Windows MATLAB`。

**鉴定：** 这不是架构错误，而是架构规则缺少一个明确注册的例外条款。没有这个例外，每个后来的 agent 都会在此矛盾上浪费时间。

### 判断 3：抽象层数量超过当前阶段的信息量

同一个 IMU 样本要穿过 7 层表示（AirSim API → UDP JSON → `sensor_msgs/Imu` → `mapping.py` → `flightcore_msgs/Imu` → ROS2Subscribe → `IMU_BUS`），并有 4 个"唯一事实源"（UDP schemas、observation registry、topics.yaml、BusConfig）。

**鉴定：** 其中两层是必要边界：`IMU_BUS`（内核事实层）和 `/uav/* flightcore_msgs`（Gateway 投影）。UDP JSON 是 Windows/WSL 物理边界，必要。但 `/aircraft/*` 作为"公共契约层"是重复的——UDP packet 协议本身已经是 simulator-neutral 的，`/aircraft/*` 再做一次 simulator-neutral 抽象，语义增量只有"以 ROS2 topic 形式可见"。

### 判断 4：第一性原理排序倒挂——时间/序号/新鲜度是最差的一环

**依据：** `interface_contract.md` 自己规定"新样本检测必须使用 Timestamp 或 Sequence，不得依赖测量值变化"，但当前实现 IsNew 未使用、输出 sequence 常数 0；`observation_protocol.md` 定义 `timestamp` 为 "Sender wall-clock seconds, Unix epoch"——即全链路存在三个时钟域（Windows 仿真墙钟 → WSL ROS time → Simulink 仿真时间），没有任何文档定义它们的映射与容差。对飞控，时序错误比字段错误致命；对世界模型数据基座，时间戳不可信的 episode 数据是废数据。

### 判断 5：整个项目在"可复现"这个第一性要求上有两个零分项

没有版本控制、没有任何 episode/日志格式定义。

**依据：** 实测仓库根目录无 `.git`；全部文档中搜不到任何 episode record、rosbag2 记录约定、run manifest 的定义。路线图 §9 的五个词里，"可复现、可记录、可比较"三个当前为 0。**这比任何架构分层问题都严重，而且修复成本最低。**

---

## 三、架构合理之处

| 项 | 依据 |
|---|---|
| FlightCore 内核不依赖仿真器/ROS2/DDS API 的原则，且有自动化检查兜底（RUNTIME_ADAPTER_ISOLATION_CONTRACT_PASS） | `flightcore_runtime_isolation.md`、handoff 验证记录 |
| truth/state 不得作为控制闭环传感器输入，GPS fallback 显式标记 debug-only 且默认关闭 | bridge README、handoff implemented 节 |
| actuator 主路径是 normalized motor setpoint，拒绝把 FlightCore 输出退化为 throttle/RPY | `observation_protocol.md` Actuator Packet 节 |
| `topics.yaml` 单一事实源 + "契约变更先加测试再改模型"的变更规则 | `interface_contract.md` 变更规则节 |
| observation registry 的 evidence rule（每个信号必须有官方 API 证据，否则 unsupported） | `simulator_observation_inventory.md` Evidence Rule |
| WSL workspace 收敛到 3 包、删除 gateway 包、拒绝 /mnt/d 构建 | handoff 2026-07-07 清理记录 |

---

## 四、架构冲突或过度设计

| 项 | 性质 | 依据 |
|---|---|---|
| UDP-only 边界规则 vs Windows MATLAB 跑 ROS2 节点 | **冲突，未言明** | 判断 2 |
| `/aircraft/*` 作为第四层公共契约 + naming norm + 13 观测族 | **过早**（V3/V4 内容提前到 V0 未完时） | 判断 1、3 |
| Windows/WSL 双份 bridge 源码树，schemas 已漂移（3 vs 13） | **冲突**（无单一权威） | 实测两侧 schemas/ |
| `flightcore_runtime_isolation.md` 中 PegasusAdapter 为 preferred、AirSim 为 backup，但全部实际工作在 AirSim | **文档-现实漂移** | isolation doc vs UAVSingle.md 全部下一步 |
| legacy 三件套并存：control packet、/aircraft/control、/aircraft/cmd_vel、legacy gateway 路径文档 | **可降级**（smoke 有用，但文档地位过高） | bridge README Legacy 节 |
| EscCmd vs MotorSetpoint 改名讨论、SystemHealth.gateway_status 改名候选 | **过早优化**（闭环没跑通前改名是纯成本） | WSL README、handoff residual |

---

## 五、技术栈风险

| 项 | 依据 |
|---|---|
| **AirSim 已被微软归档停止维护**（2022 年归档，社区 fork 为 Colosseum），`moveByMotorPWMsAsync` 支持性未实测——整条 actuator 主路径押在一个未验证 API 上 | UAVSingle.md 风险节自己承认未确认 |
| Windows MATLAB + WSL2 ROS2 + 跨界 DDS best-effort QoS：三时钟域、NAT 模式、防火墙/IP 每次变化——smoke 一直没跑通，环境摩擦是主要嫌疑 | 3_Integration/ROS2/README.md QoS 注记 |
| MATLAB ROS Toolbox 自定义消息生成链条脆弱（需要独立 conda Python 3.10、临时 MATLAB home 绕 ros2genmsg bug） | 3_Integration/ROS2/README.md 末节 |
| ROS2 Jazzy 当前承担的角色（PlotJuggler/rosbag2/生态校准）按路线图 §8 属于 V4"参照物"，现在却坐在数据主通路上 | 路线图 §8 vs bridge README 主链路 |

---

## 六、未考虑事项

1. **时钟域映射**：三个时钟域（Windows 墙钟、WSL ROS time、Simulink 仿真时间）无定义、无容差。
2. **无版本控制、无 episode 格式**（判断 5）。 
3. **FlightCore_ROS2_loop 的仿真时间 vs ROS time**：Simulink 模型以什么步长/触发方式消费 topic？丢包/乱序时估计器行为？"过期数据策略"（stale N ms 后如何降级）没有定义。
4. **generated code 的最终宿主**：如果未来 FlightCore 生成 C++ 跑在 WSL/嵌入式，那 Windows MATLAB ROS 路径只是开发期脚手架——这个"脚手架"定位从未写明，导致它被当成主线维护。

---

## 七、裁决与落地

### A. 项目主线

**裁决（2026-07-07）：** Simulink-first 飞控实验基座，DDS 跨界例外为已注册的临时脚手架。

**当前阶段一句话主线：**
> 在真实的 AirSim→WSL→FlightCore 外部闭环中，跑出第一个"带 manifest 的可回放悬停 episode"——在此之前不新增任何接口、消息、观测族或改名。

### B. DDS 跨界例外

**分阶段方案：**

- **现在（开发期）：** FlightCore_ROS2_loop 在 Windows MATLAB 运行，/uav/* 六 topic 允许跨界 DDS best-effort。UDP-only 规则仅约束 simulator endpoint 路径。
- **触发条件（全部满足后换防）：** ① 外部闭环已验证 ② 控制逻辑进入低频修改 ③ M6 代码生成启动
- **终态：** FlightCore 生成代码迁入 WSL，例外条款自动废止，UDP-only 成为 UNIVERSAL 规则。

### C. 文件体系治理

| 原则 | 内容 |
|------|------|
| ① 每个事实只有一个权威文件 | UDP schemas 权威在 WSL，Windows 仅 vendor |
| ② 文档分四层 | Tier 0 思想（vision/）→ Tier 1 契约（contracts/）→ Tier 2 手册（组件内）→ Tier 3 状态（仅 PBOS）|
| ③ 仓库不存状态 | 完成度、进度、遗留项只在 PBOS handoff |

### D. 冻结清单

| 冻结项 | 理由 |
|--------|------|
| barometer/magnetometer/rangefinder/lidar 运行时落地 | 控制需求未拉动，schema 已为 evidence 文档资产 |
| /aircraft/* naming 迁移（/aircraft/state → /aircraft/truth/state） | 纯改名，零功能增量 |
| EscCmd/SystemHealth 改名 | 闭环跑通前不动 |
| Gazebo/Isaac/Pegasus adapter | 各自的里程碑未启动 |
| MAVLink Gateway | M5 未开始 |
| 任何新增 /uav topic | 六 topic 冻结至 V0 验证通过 |
| 任何 RL/视觉/world-model 接口实现 | 路线图 V3 |
| C++ FlightBus 中间件 | superpower 时期遗留，已推迟 |

---

## 八、下一步最小行动序列

| # | 行动 | 判据 |
|---|------|------|
| 0 | 文件治理 + 两侧 git init | 两侧 `git log` 有首个 commit（已完成） |
| 1 | Mock 端到端 smoke（DDS 例外已生效） | mock 日志出现 actuator packet；三时钟域偏差实测落盘 |
| 2 | AirSim `moveByMotorPWMsAsync` + 电机序探针 | 四电机按预期响应，`--motor-order` 定值 |
| 3 | 真实 AirSim 悬停 30s 外部闭环 + rosbag2 + manifest + 判据脚本 | 可回放、带判据结果的外部 episode |
| — | 附着在 #3 内：sequence 递增、health/status 发布、IsNew 策略定值 | — |

---

## 附录：完整文件清单（治理后）

```
UAVSingleFlightControl/
├── README.md                    ← 入口地图（本审计结果已融合于此）
├── CLAUDE.md                    ← Simulink MBD 操作指引（中文）
├── AGENTS.md                    ← Codex 特有配置（中文）
├── .gitignore
├── docs/
│   ├── vision/
│   │   ├── UAV_World_Model_Capability_Roadmap.md
│   │   └── 自建飞控消息系统设计思想.md
│   ├── contracts/
│   │   ├── interface_contract.md
│   │   └── flightcore_runtime_isolation.md    ← 含 DDS 例外条款
│   └── archive/
│       ├── AirSim整机闭环仿真执行计划.md
│       ├── run_runtime_gateway_wsl.sh
│       └── build_flightcore_msgs_wsl.sh
├── FC_SimulinkProject/          ← MATLAB/Simulink MBD 项目
│   ├── 1_Data_Dictionaries/
│   ├── 2_Model/
│   ├── 3_Integration/ROS2/      ← ROS2 映射 + flightcore_msgs
│   ├── 4_Test/
│   └── 5_Tool/
└── bridge/
    └── airsim_ros2_udp_bridge/  ← Windows 侧 endpoint + 4 vendored schema
```

WSL 独立 git repo：`~/uavsingle_ros2_ws/src`（三包：aircraft_udp_bridge / flightcore_msgs / flightcore_runtime_adapter）
