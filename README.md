# UAV 单机飞控系统

> 基于 MATLAB/Simulink、ROS 2 Jazzy 与 Gazebo Harmonic 的单机飞控实验基座。
> 当前唯一开发主线是 `FlightCore_Gazebo_loop` 生成 standalone ROS 2 node，
> 并在 WSL 内与 Coordinator、Gazebo 进行严格锁步联合仿真。

## 当前基线

本项目当前使用 1 kg F450 四旋翼轻量测试床验证 FlightCore 算法、代码生成和
仿真接口。30 kg 六旋翼仍是目标产品，但其质量、惯量、执行器布局和气动参数
尚未进入当前 plant，不应将本测试床结果外推为目标机数字孪生结论。

2026-07-30 完整联合仿真基线：

| 项目 | 结果 |
|---|---:|
| 固定步长 | 1 ms |
| 仿真时长 | 15 s / 15000 epoch |
| IMU | 15001 条，PRIME 后每拍一条 |
| GPS | 76 条，iteration 0、200、…、15000 |
| ObservationReady | 15001 条 |
| ActuatorCommand | 15000 条 |
| 5 s 指令 | NED 高度设定从 0 阶跃至 -5 m |
| 15 s 高度 | 5.126 m |
| 15 s NED 垂向速度 | 0.012 m/s |
| 最终电机指令 | 四路均为 0.586 |
| 调度结果 | 无 stall、abort、跳拍或随机卡死 |

5 s 是起飞并进入 5 m 位置保持的指令时刻，不是飞行器已经悬停的时刻。15 s
结果表明飞行器已在 5 m 附近进入稳定悬停。

## 当前主线

```text
Windows MATLAB / Simulink
  FlightCore_Gazebo_loop.slx
  └─ standalone ROS 2 code generation
                   │ deploy
                   ▼
WSL Ubuntu 24.04 / ROS 2 Jazzy
  generated FlightCore node
       │ IMU / GPS                   │ ActuatorCommand
       ▼                             ▼
  ObservationReady              Gazebo System Plugin
       │                             │ plant step/result
       └──── Simulation Coordinator ─┘
                         │
                         ├─ sole /clock authority
                         └─ sole WorldControl authority
```

运行体全部位于 WSL。Windows 只维护 Simulink 模型、数据字典、ROS 2 消息源
契约、测试和文档，不参与正式联合仿真的实时数据面。

## 锁步协议

每个 epoch 都必须闭合下面的因果链：

```text
/clock(N)
  -> FlightCore 读取已提交观测并计算 ActuatorCommand(N+1)
  -> Gazebo 缓存命令并执行精确一个物理步
  -> Coordinator 发布 CommitRelease(N+1)
  -> Gazebo 发布本拍 IMU/GPS
  -> Gazebo 发布 SensorBatchPublished(required_mask)
  -> FlightCore 异步接收必需传感器
  -> FlightCore 发布 ObservationReady(received_mask)
  -> Coordinator 同时等到 ObservationReady 与 step_notify
  -> /clock(N+1)
```

传感器位定义：

```text
IMU = 0x01
GPS = 0x02
```

- PRIME 和 `iteration % 200 == 0`：`required_mask = IMU | GPS`。
- 其他 iteration：`required_mask = IMU`。
- GPS 周期只由 Gazebo 计算并随批次发布；FlightCore 和 Coordinator 不复制
  `gps_rate_divider`，避免多方配置漂移。
- 非 GPS 更新拍继续保持上一份 GPS 数据，同时模型订阅块 `IsNew=false`。
- 必需消息缺失时禁止用旧消息冒充；Coordinator watchdog 输出缺失状态并停止
  整个联合仿真。

这套消费端屏障取代了旧 `SensorReleased`。生产端完成 DDS `publish()` 并不
等于 generated FlightCore 已执行订阅回调，因此下一拍只能由
`ObservationReady` 授权。

## 源码所有权

### Windows 仓库

`D:\Project\UAVSingleFlightControl`

- `FC_SimulinkProject/1_Data_Dictionaries/`：Bus 与参数权威源。
- `FC_SimulinkProject/2_Model/`：EKF、控制和动力系统模型。
- `FC_SimulinkProject/3_Integration/FlightCore_Gazebo_loop.slx`：Gazebo
  专属 harness。
- `FC_SimulinkProject/3_Integration/ROS2/flightcore_gazebo_msgs/`：Gazebo
  锁步消息源契约。
- `FC_SimulinkProject/4_Test/`：模型与接口契约测试。

### WSL 仓库

`/home/hustle/uavsingle_ros2_ws`

- `src/packages/common/`：通用 FlightCore ROS 2 消息。
- `src/packages/gazebo/flightcore_gazebo_loop/`：generated FlightCore node。
- `src/packages/gazebo/flightcore_simulation_coordinator/`：锁步 Coordinator。
- `src/packages/gazebo/flightcore_gazebo_system/`：Gazebo System Plugin、world
  和 launch。
- `src/packages/gazebo/flightcore_gazebo_msgs/`：WSL 构建使用的锁步接口。
- `src/scripts/`：WSL 权威运行与就绪检查入口。

不要从 `/mnt/d` 长期构建 ROS 2 包，也不要恢复 Windows `bridge/wsl` 镜像为
源码权威。

## AirSim 路线：冻结

AirSim 路线已经降为冻结历史资产，不再是当前开发、部署或验收主线：

- `FlightCore_ROS2_loop.slx` 只保留兼容和历史参考，不继续扩展。
- Windows AirSim UDP endpoint、`/aircraft/*`、`/uav/*`、CycloneDDS 跨
  Windows–WSL 数据面均不参加当前 Gazebo 联合仿真。
- 不继续维护 AirSim bridge、runtime adapter、episode orchestration 或新接口。
- 冻结代码不得反向进入 FlightCore 核心，也不得作为 Gazebo 实现的依赖。
- 只有新的明确项目裁决、独立验收目标和测试计划同时具备时，才能解冻。

删除旧路径属于路线冻结与权威迁移的一部分；需要历史实现时从 Git 历史读取，
不要重新复制回现行目录。

## 快速开始

### 1. 打开 MATLAB 工程

```powershell
Start-Process -FilePath 'D:\MATLAB\R2025b\bin\matlab.exe' `
  -ArgumentList '-desktop' `
  -WorkingDirectory 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject'
```

```matlab
openProject('D:\Project\UAVSingleFlightControl\FC_SimulinkProject\FC_SimulinkProject.prj')
```

### 2. 构建 WSL 工作区

当前工作区采用 isolated install 布局：

```bash
cd ~/uavsingle_ros2_ws
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash
```

### 3. 一键启动联合仿真

```bash
cd ~/uavsingle_ros2_ws
./src/scripts/run_flightcore_gazebo_runtime.sh --epochs 15000
```

脚本负责启动 paused Gazebo、generated FlightCore、Coordinator 和
PlotJuggler，等待完整 ROS 图匹配后再执行 PRIME 与启动门。脚本使用
PlotJuggler 3.17.2 的 `--start_streamer` 自动恢复布局中保存的 ROS 2 话题；
检测到全部实时订阅已建立后才推进仿真，因此不依赖人工确认框，也不会漏掉
PRIME 与第 0 拍数据。布局同时启用 PlotJuggler 原生的无限流式缓存
（界面显示 `=inf`），不会按最近若干秒裁剪早期样本；四张图在每次数据刷新时
自动缩放到从本次仿真首个样本到当前样本的完整时间范围。

默认 PlotJuggler 面板显示：

- GPS 高度与 NED 垂向速度；
- 四路执行器命令；
- 三轴 IMU 加速度；
- ObservationReady 的 step、iteration 与传感器 mask。

每次运行在 `~/uavsingle_runs/<时间>_session_<ID>/` 下生成互相独立的日志：

- `orchestrator.log`：启动顺序、就绪门、PRIME、启动和最终判定；
- `simulation.log`：ROS 2 launch、generated node、Coordinator 与 Gazebo
  的完整运行日志；
- `plotjuggler.log`：PlotJuggler 和 ROS 2 streaming 插件日志；
- `run_metadata.env`：session、epoch、timeout 与本次运行目录。

默认情况下，当前终端只显示精简的中文编排状态，脚本另外打开“联合仿真日志”
和“PlotJuggler 实时数据日志”两个独立控制台窗口。自动化或无桌面运行可增加
`--no-log-windows`。“联合仿真日志”窗口每 1 秒显示一次最新锁步进度；
`simulation.log` 文件仍保存全部逐 epoch 原始消息。

编排入口和两个就绪探针自产的提示、错误与状态日志统一使用中文。
`simulation.log` 和 `plotjuggler.log` 同时保留 ROS 2、Gazebo、generated node
及 PlotJuggler 的原始第三方日志，避免翻译或过滤破坏诊断证据。

常用无界面/自动清理运行：

```bash
./src/scripts/run_flightcore_gazebo_runtime.sh \
  --epochs 1000 --no-gui --no-plotjuggler

./src/scripts/run_flightcore_gazebo_runtime.sh \
  --epochs 15000 --close-plotjuggler-on-exit
```

成功日志必须包含：

```text
PRIME_OBSERVATION_READY ... required_mask=3 received_mask=3
OBSERVATION_READY_ACK ... iteration=200 required_mask=3 received_mask=3
COORDINATOR_COMPLETE epochs=15000 final_sim_time_ns=15000000000
```

任何 `COORDINATOR_STALLED` 或 `COORDINATOR_ABORTED` 都表示本次运行失败。

## 重要维护约束

1. FlightCore 核心不得出现 Gazebo、ROS 2、DDS、UDP 或仿真器 API。
2. 参数只通过数据字典与参数源维护；Bus 只在 `BusConfig/` 定义。
3. 接口变化先更新 `4_Test/` 契约测试，再改消息、模型或部署代码。
4. Coordinator 是唯一 `/clock` 与 Gazebo WorldControl 权威。
5. 传感器到达与传感器有效性是两个概念；无效消息可以完成传输屏障，但由
   FlightCore health/control 逻辑决定是否继续控制。
6. `build/`、`install/`、`log/`、`slprj/`、`derived/`、`__pycache__/` 和
   episode 产物不得进入 Git。
7. `flightcore_gazebo_loop` 中的 runtime wrapper 来自 Simulink codegen；
   当前异步订阅补丁在重新 codegen 后可能被覆盖，重新生成后必须复核
   `slros2_generic_pubsub.h` 与 `ros2nodeinterface.cpp`。

详细操作规则见 [CLAUDE.md](CLAUDE.md)，文档索引见
[docs/README.md](docs/README.md)，会话状态与交接以 PBOS 为准。
