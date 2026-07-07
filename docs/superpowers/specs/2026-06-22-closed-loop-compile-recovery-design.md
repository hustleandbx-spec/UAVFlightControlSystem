# UAV_FC_loop 闭环编译恢复设计

## 目标

在不引入 AirSim、不切换状态估计算法的前提下，使 `UAV_FC_loop.slx` 完成更新编译，并把发现的问题固化为可重复执行的回归测试。

## 范围

- 固定使用 `EKF.slx`，ESKF/UKF 不进入本轮集成。
- 保持顶层固定步长 `0.001 s`；`UAV_FlightControl.slx` 和 `EKF.slx` 使用 `FixedStepDiscrete`，引用模型步长保持 `auto` 继承。
- 按顶层更新编译返回的首个错误逐项修复，不并行扩展 IMU/GPS 功能，也不接入 AirSim。
- 仅修改解除当前闭环编译阻塞所必需的模型、参数源、构建脚本和测试。

## 执行流程

1. 运行 `test_ekf_integration`，更新编译 EKF、FlightControl 和 `UAV_FC_loop`。
2. 若失败，记录首个根因，优先检查模型引用、Bus 类型/尺寸、采样时间和数据字典参数解析。
3. 为根因增加或扩展 MATLAB 回归测试，使修复前失败、修复后通过。
4. 实施最小修复并重新运行相关单体测试。
5. 再次更新编译 `UAV_FC_loop`；重复上述过程，直到编译通过或遇到需要用户决定的架构问题。

## 验收标准

- `test_flight_control_integration` 通过。
- `test_ekf_integration` 通过，且其中 `UAV_FC_loop` 更新编译成功。
- `test_sensor_sample_time_contract` 通过，确保传感器采样时间契约不回归。
- Control、EKF 和顶层模型的求解器配置保持一致。
- 不修改 ESKF/UKF，不添加 AirSim 代码，不直接手工修改 `.sldd` 条目。

## 后续边界

闭环更新编译通过后，再进入短时悬停仿真与自动判据；IMU AccelTrue、GPS 参数收口和 AirSim 接入分别作为后续独立任务处理。
