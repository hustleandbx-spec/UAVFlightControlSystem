# FlightCore–Gazebo 原生 ROS 2 接口

`FlightCore_Gazebo_loop.slx` 是 Gazebo 专属 harness。FlightCore 核心模型保持
仿真器无关；ROS 2 与 Gazebo 适配只位于本 harness 和 WSL runtime 边界。

模型顶层使用 MathWorks 原生 ROS 2 块：

- Reliable Subscribe `/flightcore/gazebo/imu`，类型 `flightcore_msgs/Imu`
- Reliable Subscribe `/flightcore/gazebo/gps`，类型 `flightcore_msgs/Gps`
- Reliable Publish `/flightcore/gazebo/actuator_command`，类型
  `flightcore_gazebo_msgs/ActuatorCommand`

IMU 与 GPS 消息分别直接映射到 FlightCore 的 `IMUBus` 和 `GPSBus`。模型中
没有 StepResult、StepResultAdapter、接收门控或 Level-2 MATLAB ROS 2
S-Function。执行器事务的 step/iteration 字段由 harness 执行计数产生。

Gazebo 插件在 PRIME 时发布 iteration 0 的 IMU/GPS；每次 Coordinator
`CommitRelease` 后发布本步 IMU，并在 `gps_rate_divider` 到期时发布 GPS。
session 身份只保留在 Coordinator–Gazebo 控制面，不进入传感器或执行器数据面。

旧的 Desktop participant 实现和运行入口已经删除。正式联合运行必须等待
generated FlightCore ROS 2 node 与 ROS 2 time model stepping 的 WSL 入口
完成；不得恢复旧入口绕过这一约束。
