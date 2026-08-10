# 时间契约架构审查结论

> 日期：2026-07-20
> 参与：本对话
> 范围：审查 `TIME_CONTRACT_ARCHITECTURE.md` 可行性、与当前模型的匹配度、Simulink+Gazebo 联合仿真的执行路径

---

## 审查要点

### A. 契约设计本身可行性：✅ 成立，但属于重型确定性回归方案

- 语义自洽，无因果漏洞
- 核心理念（稀疏 SensorBatch、available_time 释放、MODELED_DROP 区分）正确
- 代价：单 epoch 串行事务 → 运行速度远低于实时；
  Coordinator 状态机 + 事件账本 + 规范化 hash → 数周数月工程量
- 适用场景：**确定性回归验证床**，不适用于日常快速开发迭代

### B. 与当前模型的匹配度

**匹配项：**
- `SE_IMU_SAMPLE_TIME = 0.001` / `TC_RATE_CONTROL_TS = 0.001` 与契约 ΔT_epoch=1ms 一致
- IMU_BUS/GPS_BUS 已有 `Valid`/`Timestamp`/`IsNew` 字段
- InterfaceContract 已禁止按测量值变化判新
- FlightCore 无仿真器/ROS2 符号（规则 6）

**缺口（4 项）：**

| # | 缺口 | 严重程度 |
|---|---|---|
| 1 | EKF 当前无 OOSM（延迟观测融合）能力；初版冻结 L_*=0 则可绕开 | ⚠️ 合约初版可绕行 |
| 2 | CommandInput（FlightCmdBus）在契约中完全缺失；动态指令会破坏确定性 | ⚠️ 需补条款 |
| 3 | FlightCoreNode 执行模型：Simulink desktop 不适合 epoch 单步；正确形态是代码生成+薄外壳 | 🔴 实施前必须明确 |
| 4 | 契约建立在 Gazebo 之上，但 Gazebo adapter 处于 P4 冻结中 | ⚠️ 需加门控声明 |

### C. 与冻结清单的冲突

DEVELOPMENT_PLAN.md 明确冻结「Gazebo、Isaac、Pegasus adapter」直到 P4 完成。
结论：契约目前只能作为**设计文档存在**，实现受 P4 解冻门控。
文档当前标注「状态：已确认」→ 建议改为「状态：已确认（设计），实现受 P4 解冻门控」。

---

## 核心矛盾与解决路径

### 矛盾：需要 Gazebo 的物理真实感，又要在 Simulink 里调算法，又必须时间同步

放弃纯 MIL（物理太假）和全量 Coordinator（太重太慢）这两个极端。
解决方案：**两阶段分层**——开发期轻量锁步 + 回归期确定性合同。

### 阶段一：CoSimDriver 轻量锁步（开发期）

```
Gazebo Harmonic（paused）
  ← WorldControl/multi_step（Simulink 驱动）
  → SensorBatch via ROS2（IMU@1kHz, GPS@5Hz）
  → Simulink 每步阻塞等待传感器 stamp ≥ t_target
  → 时间轴焊死：无幻影延迟，非确定性只来自 DDS 抖动
  无 Coordinator / 无事件账本 / 无 hash → 约 100 行 System block
```

**技术基础设施：**
- ROS2 Jazzy + Gazebo Harmonic（已就绪）
- CycloneDDS 跨界通道（P1 已验证）
- WorldControl service via ros_gz_bridge
- CoSimDriver 为 MATLAB System block，位于 harness 层（FlighCore 规则 6 不受影响）
- 总线语义（稀疏批次/IsNew/原始 sample_time）与契约一致

**已知妥协（诚实记录）：**
1. 命令送达竞态：send()→step() 之间理论上微秒级竞态窗口
2. 卡死无归因：无法区分 Gazebo 崩溃/bridge 断开/超时
→ 这两条由全量 Coordinator 在回归期解决

### 阶段二：全量时间契约（回归期）

- FlightCore 代码生成 + C++ ROS2 薄外壳 = FlightCoreNode
- Coordinator 增加审计层（事件账本、payload hash、generation、MODELED_DROP 归因）
- 消息 schema 和 epoch 语义与阶段一共享 → 迁移平滑

### 关于 Gazebo Pacer（MathWorks 方案）

- 官方 Gazebo Co-Simulation 框架（含 Pacer 块）**不支持 Gazebo Harmonic**，仅支持 Gazebo Classic 9/10/11
- 你的环境是 Gazebo Harmonic + ROS2 Jazzy，因此需要自建 CoSimDriver
- gz-classic 已 EOL，为 Pacer 降级不值得
- Pacer 的**概念**（paused → Simulink 驱动步进 → 阻塞等待结果）完全复用

---

## 建议落地顺序

1. WSL 侧：World SDF（paused, max_step_size）, sensor 配置, motor 插件, ros_gz_bridge → 手工验证 stepping 链
2. MATLAB 侧：脚本版 CoSimDriver 跑通 100 epoch 握手 → 测单步往返墙钟耗时
3. Simulink harness：封装为 System block，FlightCore 引用模型接入 → 跑 30s 悬停
4. 测试先行（规则 8）：4_Test/ 补充 Timestamp/IsNew/稀疏批次断言
5. TIME_CONTRACT_ARCHITECTURE.md 补充「开发模式」章节

---

## 对文档的修订建议

- 标记为「回归验证规范」，加开发模式声明
- 补充 CommandInput 确定性通道条款
- 初版明确 `availability_delay = 0`（EKF 无需 OOSM）
- FlightCoreNode 实施声明：生成代码 + ROS2 外壳，非桌面 Simulink
- 添加实现门控条款（受 P4 解冻）
