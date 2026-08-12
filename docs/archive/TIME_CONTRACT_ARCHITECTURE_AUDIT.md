# UAVSingleFlightControl 时间契约架构审计报告

> 审计日期：2026-07-20  
> 审计对象：`TIME_CONTRACT_ARCHITECTURE.md` 及当前仓库现状  
> 审计方式：只读静态审计、仓库状态检查、现有 Python 协议测试  
> 边界：本报告只记录审计结果，不代表问题修订方案已获批准，不修改原时间契约、模型或运行时代码

## 0. 后续修订追踪

> 更新日期：2026-07-20  
> 说明：本节记录审计后的处置状态；第 2 节保留审计发生时的原始发现，不据此改写历史证据。

| 发现 | 状态 | `TIME_CONTRACT_ARCHITECTURE.md` 中的处置 |
|---|---|---|
| 2.1 多速率传感器延迟与单 epoch 屏障冲突 | 已解决 | 使用每个目标 epoch 一个稀疏 SensorBatch；采样 epoch、可用时间和目标 epoch 分离；延迟帧在首个允许消费的目标 epoch 释放 |
| 2.2 `ACTUATOR_LATCHED` 与 Gazebo PreUpdate 不一致 | 已解决 | 拆分为推进前 `ACTUATOR_COMMAND_ACCEPTED` 和由 PreUpdate/PostUpdate 证明的 `ACTUATOR_APPLIED`；WorldControl 响应不作为物理提交证据 |
| 2.3 Coordinator 缺少 payload hash | 已解决 | EventReport 增加 `payload_hash`；Coordinator 交叉核对数据发送端与接收端报告；EventKey 和重复规则统一包含 `producer_id` |
| 2.4 当前 epoch 与下一 SensorBatch 账本归属不明确 | 已解决 | EventReport 区分 `transaction_epoch_id` 与 `subject_epoch_id`；下一 SensorBatch 事件归属事务 `k`，数据目标为 `k+1` |
| 2.5 时间契约尚未成为仓库权威主线 | 待处理 | 等本轮协议修订审核通过后，再移动到 `docs/contracts/` 并同步 README、开发计划和 runtime isolation |
| 3.1 InterfaceContract 落后于 Bus | 待处理 | 尚未同步 `IsNew`、`ArmRequest`、`Armed`、`Valid` |
| 3.2 Runtime isolation 测试路径失效 | 待处理 | 当前工作树中原测试已删除，需以现行契约路径重建回归测试，不能把删除视为覆盖恢复 |
| 3.3 Agent 启动指针失效 | 待处理 | 尚未清理 `.claude/skills/simulink-mbd.md` 失效入口 |

本轮只修订协议文档和本审计追踪，没有实现 Gazebo 插件、Coordinator、ROS 2 消息、FlightCore 外壳或 Simulink 模型。2.2～2.4 的关闭表示文档内语义已经闭合，不表示运行时实现已经通过验证。

## 1. 总体结论

三节点架构的基本方向成立：GazeboNode 负责物理状态，FlightCoreNode 负责飞控计算，SimulationCoordinatorNode 负责 epoch 事务和提交屏障；高频数据面不经过 Coordinator，也符合降低耦合和避免中心节点搬运 payload 的目标。

但当前文档尚不能作为可直接实施的协议基线。审计发现两个会阻塞协议执行的问题，以及若干事件身份、仓库权威性和测试基线问题。当前更准确的成熟度判断是：

```text
架构方向已形成；协议闭环、Gazebo 时序可实现性和仓库主线切换尚待审核。
```

## 2. 关键发现

### 2.1 [P0] 多速率传感器延迟与单 epoch 屏障存在冲突

文档已经定义不同传感器具有不同采样频率，也区分了 `t_sample` 和 `t_available`。但正常 epoch 的基准屏障仍统一要求：

```text
NEXT_SENSOR_BATCH_COMMITTED
NEXT_SENSOR_BATCH_RECEIVED
```

单 epoch 流程又默认每次物理推进结束都会生成一个供下一 epoch 使用的 `SensorBatch[k+1]`。

GPS 示例规定：

```text
t_sample    = 200 ms
t_available = 230 ms
```

由此产生未闭合的事件归属问题：

- 如果 GPS 接收事件属于 200 ms 对应的 epoch，该 epoch 必须等待到 230 ms 才能提交；但物理时间未提交又不能继续推进到 230 ms，形成死锁。
- 如果 GPS 接收事件属于 230 ms 对应的 epoch，当前契约没有定义采样 epoch、可用 epoch、目标 epoch 与事件账本之间的映射。
- 文档虽然说明各 epoch 的预期传感器集合不同，但 `SensorBatch` 和屏障模型仍隐含了各 epoch 具有同构内容的假设，没有明确表达“本 epoch 新采样的数据”和“以前采样、到本 epoch 才允许消费的数据”是两组不同事件。

该问题必须先经过独立方案审核，再修改时间契约。当前报告不裁决具体修订方式。

### 2.2 [P0] `ACTUATOR_LATCHED` 的位置与 Gazebo PreUpdate 时序不一致

当前时序要求：

```text
ActuatorBatch(k)
→ ACTUATOR_LATCHED(k)
→ ADVANCE_PHYSICS(k)
```

但 Gazebo 执行器对物理组件的实际写入通常发生在被推进 step 的 `PreUpdate`。如果 `ACTUATOR_LATCHED` 表示真正写入执行器模型，就必须先推进 step，当前顺序会形成等待环；如果它仅表示 ROS 回调收到并缓存命令，则事件名称和 `t_actuator_latch` 的物理语义不准确。

PBOS 项目交接中已有相关纠正记录，但尚未进入正式时间契约。

### 2.3 [P1] Coordinator 无法执行 payload hash 冲突检查

`EventRecord`、重复消息规则和验收标准都依赖 `payload_hash`，同时文档规定 Coordinator 不接收和保存高频 payload。但是 `EventReport` 接口没有 `payload_hash` 字段。

因此 Coordinator 当前无法独立判断：

```text
同一事件身份 + 相同 payload
同一事件身份 + 冲突 payload
```

另外，`EventKey` 包含 `producer_id`，重复消息判定字段却遗漏了 `producer_id`，两处唯一身份定义不一致。

### 2.4 [P1] 当前 epoch 与下一 SensorBatch 的账本所有权不明确

epoch `k` 的屏障期待 `NEXT_SENSOR_BATCH_*`，流程中的报告却携带 `k+1`；与此同时，未来消息规则默认拒绝超出当前窗口的消息。

目前没有明确规定事件报告中的 `epoch_id` 表示：

- 触发该事件的事务 epoch；
- 数据所属的采样 epoch；
- 数据允许被消费的目标 epoch；
- 或物理推进完成后的状态 epoch。

该歧义会直接影响事件匹配、未来消息校验、幂等处理和历史账本归档。

### 2.5 [P1] 时间契约尚未成为仓库权威主线

审计时确认：

- `TIME_CONTRACT_ARCHITECTURE.md` 尚未被 Git 跟踪；
- 文档位于仓库根目录，尚未进入 `docs/contracts/` 的权威索引；
- 仓库中没有对应的 Coordinator、GazeboNode、FlightCoreNode 协议外壳实现；
- 没有 `ExecuteEpoch`、`AdvancePhysics`、`SensorBatch`、`ActuatorBatch`、`EventReport` 对应的 ROS 2 消息或服务定义；
- 没有 generation、epoch、屏障、重复事件和故障冻结的协议测试。

同时，当前仓库权威文档仍保留旧主线：

- `README.md` 仍以真实 AirSim 外部闭环 episode 为当前目标；
- `DEVELOPMENT_PLAN.md` 仍冻结 Gazebo adapter；
- `CLAUDE.md` 仍规定 AirSim episode 通过前不启动 Gazebo；
- `docs/contracts/flightcore_runtime_isolation.md` 仍把 AirSim 定义为当前外部 simulator adapter。

因此当前存在：

```text
PBOS / 新时间契约：Gazebo 为主 Plant 路线
仓库入口 / 开发计划 / 运行时契约：AirSim 仍为当前主线
```

## 3. 接口与测试一致性发现

### 3.1 [P1] InterfaceContract 落后于当前 Bus 定义

当前 Bus 配置已经包含：

- `IMU_BUS.IsNew`
- `GPS_BUS.IsNew`
- `FlightCmdBus.ArmRequest`
- `EscCmdBus.Armed`
- `EscCmdBus.Valid`

但 `docs/contracts/interface_contract.md` 没有完整记录这些字段。新 FlightCoreNode 协议外壳若直接依据该文档实现，将与当前模型边界不一致。

### 3.2 [P1] Runtime isolation 契约测试引用了失效路径

`FC_SimulinkProject/4_Test/test_runtime_adapter_isolation_contract.m` 仍读取：

```text
docs/architecture/flightcore_runtime_isolation.md
```

实际文件已经迁移到：

```text
docs/contracts/flightcore_runtime_isolation.md
```

因此该测试按当前内容必然失败，不能作为有效架构回归门。

### 3.3 [P2] Agent 启动指针失效

`CLAUDE.md` 要求会话开始时读取：

```text
.claude/skills/simulink-mbd.md
```

该文件当前不存在，启动规则无法完整执行。

## 4. 仓库状态

审计时仓库状态如下：

```text
branch: master
HEAD:   67daaa6
HEAD 日期: 2026-07-09
```

工作树包含大量未提交变更：

- 46 个 tracked 文件发生修改或删除；
- 多个 `.slx`、`.sldd`、Bus 定义、ROS 2 消息和脚本处于修改状态；
- 存在未跟踪 harness、测试、日志、工具和时间契约文档；
- 当前状态尚未形成可复现的 Gazebo 时间契约基线。

仓库根 `.gitignore` 忽略了名为 `log/` 的目录，但没有统一忽略 `*.log`，因此 `endpoint.log`、`matlab_test.log` 等运行产物仍会出现在未跟踪文件中。

## 5. 验证记录

本次执行了 Windows AirSim UDP 协议测试：

```text
python -B -m unittest discover -s bridge\windows\tests -v
```

结果：

```text
Ran 26 tests
OK
```

这些测试只验证现有 AirSim UDP packet、sequence 和 runtime adapter mapping，不覆盖新时间契约，也不能证明 generation、epoch、Gazebo 固定步进、屏障或 `/clock` 提交规则成立。

本次没有运行或修改 Simulink 模型，没有启动 Gazebo，没有修改 WSL ROS 2 workspace。

## 6. 审核顺序建议

后续建议按独立审核门处理，前一项获得批准后再进入下一项：

1. 多速率传感器采样、延迟可用和 epoch 屏障关系。
2. Gazebo PreUpdate/PostUpdate 下的执行器接收、实际应用和物理提交顺序。
3. EventKey、EventReport、payload hash 与跨 epoch 数据身份。
4. PRIME、空传感器集合、建模丢样和节点重启语义。
5. 协议稳定后，再同步仓库权威文档并增加纯协议状态机测试。

在上述协议问题关闭前，不建议开始编写 Gazebo 插件、Coordinator 节点或修改 FlightCore 模型。
