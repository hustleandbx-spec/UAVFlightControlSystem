# FlightCore–Gazebo 严格锁步联合仿真

本目录是 WSL 原生 FlightCore–Gazebo runtime 的权威源码。Windows 仓库只维护
Simulink 模型、消息源契约、测试与文档；不要把本目录镜像回
`D:\Project\UAVSingleFlightControl\bridge\wsl` 后再构建。

## 包组成

| 包 | 职责 |
|---|---|
| `flightcore_gazebo_loop` | `FlightCore_Gazebo_loop.slx` 生成的 standalone ROS 2 node，以及异步传感器接收包装层 |
| `flightcore_simulation_coordinator` | 唯一 `/clock` 与 Gazebo WorldControl 权威；校验全部 epoch ACK |
| `flightcore_gazebo_system` | Gazebo Harmonic System Plugin、F450 轻量 plant、world 和 launch |
| `flightcore_gazebo_msgs` | 执行器身份、物理步完成、批次发布和消费端确认接口 |
| `flightcore` / `ekf` / `uav_flightcontrol` | Simulink 生成的引用模型代码 |

通用 IMU/GPS 消息位于 `../common/flightcore_msgs`。

## 线程与时间边界

```text
Gazebo executor thread
  publish IMU/GPS
  publish SensorBatchPublished(required_mask)
                      │
                      ▼
generated node SLMultiThreadedExecutor
  execute IMU/GPS callbacks asynchronously
  cache latest messages
  publish ObservationReady(received_mask)
                      │
                      ▼
Coordinator executor
  validate identity + masks
  wait previous +FlightCore_Gazebo_loop step_notify
  publish exactly one strictly increasing /clock
                      │
                      ▼
generated base-rate thread
  read cached messages
  mModel->step()
  publish ActuatorCommand and step_notify
```

关键约束：

1. Simulink 传感器订阅不能被 executor 跳过，否则消息只能在 `model->step()`
   中消费，而 `/clock` 又在等待消费 ACK，形成循环死锁。
2. `getLatestMessage()` 只能读取回调已经缓存的数据，不能调用
   `execute_subscription()`。
3. Gazebo 的 `SensorBatchPublished` 只证明生产端调用了 `publish()`；
   Coordinator 必须等待 FlightCore 的 `ObservationReady`。
4. `step_notify` 证明当前模型步已经返回，防止下一拍 `/clock` 覆盖尚未完成的
   base-rate；它不能替代传感器 epoch 身份。

## 多速率传感器协议

```text
IMU_MASK = 0x01
GPS_MASK = 0x02
```

`gps_rate_divider=200` 时：

| iteration | required_mask | 说明 |
|---:|---:|---|
| 0 | `IMU|GPS` | PRIME，初始化 GPS 保持值 |
| 1…199 | `IMU` | GPS 保持上一值，`IsNew=false` |
| 200 | `IMU|GPS` | 等待同 iteration 的新 IMU 和 GPS |
| 201…399 | `IMU` | 仅等待新 IMU |

只有 Gazebo 计算 `required_mask`。FlightCore 和 Coordinator 不重新计算 GPS
周期，只校验收到的 mask 和 identity。必需 GPS 缺失时不得复用旧 GPS 冒充新
数据；watchdog 会输出协议状态并停止联合仿真。

消息到达与测量有效性彼此独立：

- `received_mask`：ROS 回调是否收到这一拍的消息。
- `msg.valid`：测量内容是否有效。

无效但按时到达的消息仍完成传输屏障，FlightCore health/control 再决定是否
使用测量或停止控制。

## PRIME 与 epoch 时序

PRIME：

```text
PrimeSession(session_id)
  -> Gazebo 验证全新 iteration=0 世界
  -> IMU(0) + GPS(0)
  -> SensorBatchPublished(0, IMU|GPS)
  -> ObservationReady(0)
  -> start_coordinator
  -> /clock=0
```

常规 epoch：

```text
ActuatorCommand(N)
  -> CommandCached(N)
  -> WorldControl multi_step=1
  -> PlantStepDone(N) + ResultReady(N)
  -> CommitRelease(N)
  -> sensors(N) + SensorBatchPublished(N)
  -> ObservationReady(N)
  -> /clock(N)
```

Coordinator watchdog 发现协议无进展时输出 `COORDINATOR_STALLED`，请求 Gazebo
`/server_control stop` 并以失败退出，禁止无限卡死。

## 构建

工作区当前使用 isolated install 布局：

```bash
cd ~/uavsingle_ros2_ws
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install
source install/setup.bash
```

不要混用 `--merge-install`，除非先建立新的独立构建目录。

## 运行

```bash
ros2 launch flightcore_gazebo_system \
  flightcore_gazebo_cosim.launch.py \
  gui:=true max_epochs:=15000 progress_timeout_ms:=10000
```

另开一个已 source 的 WSL 终端：

```bash
ros2 service call /flightcore/gazebo/prime_session \
  flightcore_gazebo_msgs/srv/PrimeSession "{session_id: 2026073001}"

ros2 service call /flightcore/gazebo/start_coordinator \
  std_srvs/srv/Trigger "{}"
```

`gui:=false` 只关闭观察客户端，不改变 Gazebo Server、物理推进或锁步语义。

## 2026-07-30 验收基线

15000 epoch / 15 s 完整流程结果：

- IMU 15001 条；
- GPS 76 条；
- ObservationReady 15001 条；
- ActuatorCommand 15000 条；
- iteration 199/200/201 的 mask 分别为 `1/3/1`；
- 15 s 高度 5.126 m，NED 垂向速度 0.012 m/s；
- 四电机最终指令均为 0.586；
- 无 `COORDINATOR_STALLED` 或 `COORDINATOR_ABORTED`。

## Codegen 注意事项

`flightcore_gazebo_loop` 是 Simulink 生成包。当前
`slros2_generic_pubsub.h` 和 `ros2nodeinterface.cpp` 包含项目必需的异步订阅
补丁；重新 codegen 可能覆盖它们。每次重新部署生成包后必须复核：

- IMU/GPS 没有加入 executor 的 skipped subscriber 集合；
- `getLatestMessage()` 没有执行 ROS 回调；
- `SensorBatchPublished` 订阅和 `ObservationReady` 发布仍存在；
- 重新完成构建与至少跨 iteration 200 的锁步验收。
