# FlightCore-Gazebo 锁步联合仿真方案

## 阻塞式 ROS2 Sub（S-Function）+ 原生 Pub

> 状态：历史设计依据；当前实现以 `TIME_CONTRACT_ARCHITECTURE.md` 为准  
> 日期：2026-07-27  
> 当前实现：[BlockingStepResultSubscriber.m](../../FC_SimulinkProject/3_Integration/Gazebo/BlockingStepResultSubscriber.m)

---

## 一、问题诊断

### 已废弃路线：外部 `step()` 步进

已删除的外部 Adapter 曾使用 `step(simulationController, NumberOfSteps=1)`
在 MATLAB 脚本层逐步驱动 Simulink。**根因不是脚本写法问题，而是路线问题：**

```
MATLAB 进程                               Simulink 引擎进程
    │                                           │
    ├── step(1) ──────────────────────────────→ │  每次跨进程边界：
    │   ← 引擎初始化本步上下文                    │  序列化 + 上下文切换
    │   ← 执行块                                │  + 诊断检查
    │   ← 收尾、状态保存                         │  ≈ 10-20ms/步
    │                                           │
    ├── step(1) ──────────────────────────────→ │  再来一遍
    ...                                         │
    15000 步 × 15ms = ~225 秒纯开销
    FlightCore 真正计算时间 < 2 秒
```

**结论：** MATLAB 调用 Simulink 的进程边界开销（~10-20ms/次）是 FlightCore 单步计算量（~0.1ms）的 100-200 倍。外部步进在本质上是错误路线，不能通过优化脚本解决。

### 正确路线：引擎内阻塞

把阻塞点从**引擎外**（MATLAB 脚本 `receive()`）搬到**引擎内**（S-Function `Outputs` 内 `receive()`）：

```
引擎外步进（失败）                        引擎内阻塞（正确）
                                      Simulink 引擎进程（持续运行，不退）
MATLAB ──step(1)──→ 引擎（跑1步）        ┌──────────────────────────────┐
MATLAB ──step(1)──→ 引擎（跑1步）        │ S-Function Outputs:          │
MATLAB ──step(1)──→ 引擎（跑1步）        │   receive() ← 阻塞在 DDS    │
      ...                               │   → 数据到 → FlightCore →   │
      ↑ 边界代价 × N                    │   → Pub → 下一采样周期 →    │
                                        │   receive() ← 再次阻塞      │
                                        └──────────────────────────────┘
                                            ↑ 引擎只启动一次，无反复进出
```

| | 外部 step() 步进 | 引擎内阻塞 S-Function |
|---|---|---|
| 引擎启动次数 | N 次 | **1 次** |
| 单步开销 | ~15ms | ~0.1ms（计算 + DDS 唤醒） |
| 15000 步耗时 | ~225 秒 | ~1.5 秒（纯计算） + 网络延迟 |
| 阻塞等待期间引擎状态 | 空闲（已退出当前步） | 保持运行（线程阻塞在 DDS） |

---

## 二、最终架构

```
Simulink 引擎（只启动一次，持续运行不退）

 ┌─ FlightCore_Gazebo_loop.slx ─────────────────────────────────────┐
 │                                                                    │
 │  ┌──────────────────┐   ┌──────────────────┐                      │
 │  │ BlockingImuSub   │   │ BlockingGpsSub   │                      │
 │  │ (S-Function)     │   │ (S-Function)     │                      │
 │  │                  │   │                  │                      │
 │  │ Outputs():       │   │ Outputs():       │                      │
 │  │  receive(imuSub) │   │  receive(gpsSub) │  ← 两个阻塞点        │
 │  │  → IMU_BUS       │   │  → GPS_BUS       │                      │
 │  └────────┬─────────┘   └────────┬─────────┘                      │
 │           │                      │                                 │
 │           └──────────┬───────────┘                                 │
 │                      ▼                                             │
 │           ┌──────────────────┐                                     │
 │           │   FlightCore     │  ← ModelReference，不动             │
 │           │   (算法核心)      │                                     │
 │           └────────┬─────────┘                                     │
 │                    │                                               │
 │                    ▼                                               │
 │           ┌──────────────────┐                                     │
 │           │ EscCmdPub        │  ← Simulink 原生 ROS2 Publish 块    │
 │           │ StateEstPub      │                                     │
 │           └──────────────────┘                                     │
 │                                                                    │
 └────────────────────────────────────────────────────────────────────┘
           │ DDS 阻塞等待                      ▲ DDS 非阻塞发送
           ▼                                  │
     Gazebo: IMU/GPS ─────→ FlightCore 计算 ──→ EscCmd ──→ Gazebo 物理步进
```

---

## 三、改动范围

| 组件 | 动作 | 原因 |
|------|:---:|------|
| `FlightCore_Gazebo_loop.slx` | 新增模型文件 | 锁步场景专用 wrapper |
| `ImuSub`（原生 Subscribe 块） | **替换**为 `BlockingImuSub` | 这是阻塞点 |
| `GpsSub`（原生 Subscribe 块） | **替换**为 `BlockingGpsSub` | 这是阻塞点 |
| `FlightCore`（ModelReference） | **不动** | 中间件无关 |
| `EscCmdPub`（原生 Publish 块） | **保留** | 非阻塞发送即可 |
| `StateEstPub`（原生 Publish 块） | **保留** | 非阻塞发送即可 |
| Bus 转换层 | **保留** | 信号映射不变 |

---

## 四、阻塞 Sub S-Function 实现

### 4.1 BlockingImuSub.m

基于已验证的阻塞 `receive()` 与持久状态机制实现：

```matlab
function BlockingImuSub(block)
%BLOCKINGIMUSUB 阻塞式 IMU 订阅 S-Function
%  替代标准 ROS2 Subscribe 块。在 Outputs 中阻塞等待 Gazebo 发来的 IMU 数据。
%  不携带数据时 Simulink 线程暂停，收到消息后自动继续。

    setup(block);
end

function setup(block)
    block.NumDialogPrms = 2;        % (1) SampleTime, (2) TimeoutSec
    block.DialogPrmsTunable = {'Nontunable', 'Nontunable'};

    block.NumInputPorts  = 0;
    block.NumOutputPorts = 5;       % Accel[3], Gyro[3], Valid, Timestamp, IsNew

    % 端口维度
    block.OutputPort(1).Dimensions = 3;   % Accel
    block.OutputPort(2).Dimensions = 3;   % Gyro
    block.OutputPort(3).Dimensions = 1;   % Valid
    block.OutputPort(4).Dimensions = 1;   % Timestamp
    block.OutputPort(5).Dimensions = 1;   % IsNew

    for i = 1:5
        block.OutputPort(i).DatatypeID = 0;       % double
        block.OutputPort(i).Complexity  = 'Real';
        block.OutputPort(i).SamplingMode = 'Sample';
    end
    block.OutputPort(3).DatatypeID = 8;   % boolean
    block.OutputPort(5).DatatypeID = 8;   % boolean

    sampleTime = block.DialogPrm(1).Data;
    block.SampleTimes = [sampleTime 0];
    block.SimStateCompliance = 'DisallowSimState';

    block.RegBlockMethod('Start',      @Start);
    block.RegBlockMethod('Outputs',    @Outputs);
    block.RegBlockMethod('Terminate',  @Terminate);
end

function Start(block)
    timeoutSec = block.DialogPrm(2).Data;
    key = sprintf('%.17g', block.BlockHandle);

    state.Node = ros2node( ...
        sprintf('/flightcore_blocking_imu_%05u', randi(99999)));
    state.Subscriber = ros2subscriber( ...
        state.Node, ...
        '/uav/sensors/imu', ...
        'flightcore_msgs/Imu', ...
        'Reliability', 'reliable', ...
        'Durability', 'volatile', ...
        'Depth', 1);
    state.TimeoutSec = timeoutSec;

    localStore('set', key, state);
end

function Outputs(block)
    key   = sprintf('%.17g', block.BlockHandle);
    state = localStore('get', key);

    if isempty(state)
        error('BlockingImuSub: 未初始化');
    end

    % ===== 核心：阻塞等待 =====
    try
        msg = receive(state.Subscriber, state.TimeoutSec);
    catch e
        error('BlockingImuSub: 接收超时 (%s)', e.message);
    end

    % ===== 输出 IMU_BUS 各字段 =====
    block.OutputPort(1).Data = single(msg.accel_mps2);       % Accel
    block.OutputPort(2).Data = single(msg.gyro_radps);       % Gyro
    block.OutputPort(3).Data = logical(msg.valid);           % Valid
    block.OutputPort(4).Data = double(msg.timestamp_sec);    % Timestamp
    block.OutputPort(5).Data = true;                         % IsNew
end

function Terminate(block)
    key   = sprintf('%.17g', block.BlockHandle);
    state = localStore('get', key);

    if ~isempty(state)
        try delete(state.Subscriber); catch; end
        try delete(state.Node);       catch; end
    end
    localStore('remove', key);
end

% ===== 持久状态存储 =====
function value = localStore(action, key, newValue)
    persistent states
    if isempty(states)
        states = containers.Map('KeyType', 'char', 'ValueType', 'any');
    end

    switch action
        case 'get'
            value = [];
            if isKey(states, key)
                value = states(key);
            end
        case 'set'
            states(key) = newValue;
            value = newValue;
        case 'remove'
            if isKey(states, key)
                remove(states, key);
            end
            value = [];
    end
end
```

### 4.2 BlockingGpsSub.m

结构一致，修改点：

| 项 | BlockingImuSub | BlockingGpsSub |
|---|---|---|
| Topic | `/uav/sensors/imu` | `/uav/sensors/gps` |
| 消息类型 | `flightcore_msgs/Imu` | `flightcore_msgs/Gps` |
| 节点名 | `/flightcore_blocking_imu_*` | `/flightcore_blocking_gps_*` |
| 输出端口数 | 5 | 5（Lat, Lon, Alt, Velocity[3], Valid, Timestamp, IsNew — 实际需 7 端口） |
| 输出字段 | Accel, Gyro, Valid, Timestamp, IsNew | Lat, Lon, Alt, Velocity, Valid, Timestamp, IsNew |

---

## 五、Gazebo 侧协议

Gazebo 必须**先发传感器数据，再等控制指令**，否则首步死锁：

```python
class FlightCoreLockstepPlugin(gz.sim.System):
    def __init__(self):
        self.step = 0

    def on_initial_sensor_publish(self):
        """首步：发初始传感器数据，打破锁步僵局"""
        self.imu_pub.publish(self.read_imu())     # t=0 静止状态
        self.gps_pub.publish(self.read_gps())

    def on_lockstep_update(self):
        # 1. 阻塞等待 Simulink 控制指令
        esc_msg = self.esc_sub.receive(timeout_sec=1.0)

        # 2. 施加力、步进物理
        self.apply_motor_forces(esc_msg.motor_cmd)
        self.physics_step()

        # 3. 读取新状态、发布传感器（供 Simulink 下一步读取）
        self.imu_pub.publish(self.read_imu())
        self.gps_pub.publish(self.read_gps())

        self.step += 1
```

---

## 六、锁步时序

```
           Gazebo                               Simulink
             │                                      │
  初始化:    │── pub IMU₀, GPS₀ ──────────────────→ │ (DDS 缓存)
             │                                      │
  步 1:      │ [block 等 EscCmd]                    │ BlockingImuSub: receive() → 收到 IMU₀
             │                                      │ BlockingGpsSub: receive() → 收到 GPS₀
             │                                      │ FlightCore 计算
             │ ←────────── pub EscCmd₁ ──────────── │
             │ 施加力、物理步进                      │
             │── pub IMU₁, GPS₁ ──────────────────→ │ (DDS 缓存)
             │                                      │ BlockingImuSub: receive() → 阻塞...
             │                                      │
  步 2:      │ [block 等 EscCmd]                    │ ...收到 IMU₁
             │                                      │ BlockingGpsSub: receive() → 收到 GPS₁
             │ ←────────── pub EscCmd₂ ──────────── │
             │ 施加力、物理步进                      │
             │── pub IMU₂, GPS₂ ──────────────────→ │
             │                                      │
             ...                                    ...
```

---

## 七、与已废弃方案的对比

| | 已删除的外部 step 路线 | 已删除的 TCP 一体式路线 | **当前方案**<br>（阻塞 Sub + 原生 Pub） |
|---|---|---|---|
| 阻塞机制 | MATLAB 脚本 `receive()` | TCP `readline()` | S-Function `Outputs` 内 `receive()` |
| 引擎边界 | 每步穿越一次（~15ms 开销） | 不穿越 | **不穿越** |
| 通信通道 | ROS2（仅协调元数据） | TCP | **纯 ROS2** |
| 数据/控制分离 | 分离（Coordinator + ROS2 数据面） | 混合（TCP 一线） | **分离**（阻塞 Sub 读 + 原生 Pub 发） |
| 15000 步耗时 | ~225 秒 | 取决于 TCP 延迟 | **~1.5 秒计算 + 网络延迟** |
| 调试工具 | MATLAB 断点 | tcpdump | **ros2 topic echo/bag** |

---

## 八、关键约束与注意事项

### 8.1 QoS 必须匹配

S-Function 和 Gazebo 侧的 Sub/Pub 使用一致的 `Reliability` 和 `Durability`。建议统一：

```
Reliability: reliable
Durability: volatile
Depth: 1
```

### 8.2 超时处理

`receive()` 超时后需明确的恢复策略：

1. 超时 → 重试 3 次
2. 仍失败 → 报错并终止仿真（避免永久挂死）
3. 可选：在超时回调中记录诊断信息（当前步号、最后收到数据的时间戳）

### 8.3 两个阻塞 Sub 的读取顺序保证数据一致性

- **Gazebo 侧：** 同一 callback 内**先 pub IMU → 再 pub GPS**
- **Simulink 侧：** 模型执行顺序**先 BlockingImuSub → 再 BlockingGpsSub**
- 写入顺序 = 读取顺序 → 自然保证 IMUₙ 与 GPSₙ 来自同一 Gazebo 步
- 必要时用 Simulink `block_priority` 显式锁定执行顺序

### 8.4 首步启动顺序

1. Gazebo 先启动，发布初始传感器数据（t=0 静止状态）
2. Simulink 随后启动
3. 如果 Simulink 先启动也没关系——`receive()` 阻塞等待，只要 Gazebo 在超时时间内发第一条数据

### 8.5 禁止把 S-Function 放进 Enabled/Triggered Subsystem

阻塞语义与使能/触发的执行模型冲突——Disabled 期间 S-Function 的 `Outputs` 不被调用，但 DDS 后台线程仍在接收数据，导致时序混乱。S-Function 应放在模型顶层，按正常数据流连接。

### 8.6 不与 FlightCore 耦合

FlightCore.slx 保持纯算法模型（IMU_BUS、GPS_BUS 输入 → EscCmdBus、StateEstBus 输出），不出现 ROS2、DDS、UDP、Gazebo 等任何中间件符号。所有阻塞逻辑只在 wrapper 层。

---

## 九、文件清单

| 文件 | 说明 |
|------|------|
| `FC_SimulinkProject/3_Integration/Gazebo/BlockingImuSub.m` | 阻塞式 IMU 订阅 S-Function（新建） |
| `FC_SimulinkProject/3_Integration/Gazebo/BlockingGpsSub.m` | 阻塞式 GPS 订阅 S-Function（新建） |
| `FC_SimulinkProject/3_Integration/FlightCore_Gazebo_loop.slx` | 锁步联合仿真模型（基于 `FlightCore_ROS2_loop.slx` 改造） |
| `FC_SimulinkProject/3_Integration/Gazebo/BlockingStepResultSubscriber.m` | 当前唯一阻塞接收核心 |
| `FC_SimulinkProject/3_Integration/FlightCore/FlightCore.slx` | 算法核心（不改动） |
| Gazebo 侧 `FlightCoreLockstepPlugin` | Gazebo 插件（WSL 侧新建） |
