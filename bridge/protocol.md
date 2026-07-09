# 飞行器 UDP JSON 协议 v1

每个 UDP 数据报包含恰好一个 UTF-8 编码的 JSON 对象。数据包小而轻量，无状态；
丢包通过消费最新的有效数据包来处理。协议的权威定义及 Schema 由 WSL 端的
`aircraft_udp_bridge` 包维护。此 Windows 副本为端点侧同步副本。

## 数据包集合

| 数据包 | 方向 | 角色 |
|---|---|---|
| `state` | 仿真器端点 -> WSL2 桥接 | Truth/运动学观测，用于日志记录、监控与评估 |
| `sensor_imu` | 仿真器端点 -> WSL2 桥接 | IMU 测量源，供 `/uav/sensors/imu` 消费 |
| `sensor_gps` | 仿真器端点 -> WSL2 桥接 | GPS 测量源，供 `/uav/sensors/gps` 消费 |
| `actuator` | WSL2 运行时适配器 -> 仿真器端点 | FlightCore 归一化电机指令 |
| `control` | WSL2 桥接 -> 仿真器端点 | 仅用于调试的遗留数据包，非 FlightCore 闭环主路径 |

公共字段：

| 字段 | 含义 |
|---|---|
| `protocol_version` | 协议版本号，当前为 `1` |
| `packet_type` | 数据包类型标识 |
| `timestamp` | 发送方墙上时钟秒数，Unix 纪元 |
| `sequence` | 单调递增无符号整数，允许回绕 |
| `source_id` | 可选，多传感器场景下的来源标识 |

## State 包

方向：仿真器端点 -> WSL2 桥接。

```json
{
  "protocol_version": 1,
  "packet_type": "state",
  "timestamp": 1710000000.0,
  "sequence": 1,
  "frame_id": "map_ned",
  "child_frame_id": "aircraft_frd",
  "position": [0.0, 0.0, -2.0],
  "orientation": [0.0, 0.0, 0.0, 1.0],
  "linear_velocity": [0.0, 0.0, 0.0],
  "angular_velocity": [0.0, 0.0, 0.0]
}
```

字段语义：

| 字段 | 含义 |
|---|---|
| `frame_id` | 位置与线速度的父坐标系 |
| `child_frame_id` | 角速度所在的机体坐标系 |
| `position` | `[x, y, z]`，米，默认本地 NED 坐标系 |
| `orientation` | `[qx, qy, qz, qw]` 四元数 |
| `linear_velocity` | `[vx, vy, vz]`，米/秒，参考 `frame_id` |
| `angular_velocity` | `[wx, wy, wz]`，弧度/秒，参考 `child_frame_id` |

`state` 不是 FlightCore 的传感器输入。它仅可用于真值对比、日志记录、调试可视化及
回退实验，且必须在明确标记为仅调试用途的情况下使用。

## Sensor IMU 包

方向：仿真器端点 -> WSL2 桥接。

```json
{
  "protocol_version": 1,
  "packet_type": "sensor_imu",
  "timestamp": 1710000000.0,
  "sequence": 1,
  "source_id": 0,
  "frame_id": "aircraft_frd",
  "orientation": [0.0, 0.0, 0.0, 1.0],
  "angular_velocity": [0.0, 0.0, 0.0],
  "linear_acceleration": [0.0, 0.0, -9.80665]
}
```

字段语义：

| 字段 | 含义 |
|---|---|
| `frame_id` | IMU/机体坐标系，默认 `aircraft_frd` |
| `orientation` | `[qx, qy, qz, qw]` 四元数，由端点提供 |
| `angular_velocity` | `[wx, wy, wz]`，弧度/秒 |
| `linear_acceleration` | `[ax, ay, az]`，米/秒² |

运行时适配器负责将此数据包映射为 `flightcore_msgs/Imu` 消息并遵循 Simulink
`IMU_BUS` 约定。

## Sensor GPS 包

方向：仿真器端点 -> WSL2 桥接。

```json
{
  "protocol_version": 1,
  "packet_type": "sensor_gps",
  "timestamp": 1710000000.0,
  "sequence": 1,
  "source_id": 0,
  "geo_point": {
    "latitude": 47.641468,
    "longitude": -122.140165,
    "altitude": 122.0
  },
  "velocity": [0.0, 0.0, 0.0],
  "eph": 1.0,
  "epv": 1.0,
  "fix_type": 3,
  "time_utc": 1710000000000000,
  "is_valid": true
}
```

字段语义：

| 字段 | 含义 |
|---|---|
| `geo_point` | WGS84 纬度、经度、海拔 |
| `velocity` | `[vx, vy, vz]`，米/秒，适配器映射前的端点定义本地坐标系 |
| `eph` | 可选，水平定位不确定度 |
| `epv` | 可选，垂直定位不确定度 |
| `fix_type` | 可选，GPS 定位质量，`0..3` |
| `time_utc` | 可选，来自仿真器/GPS 源的 UTC 时间 |
| `is_valid` | 测量有效性标志 |

运行时适配器将此数据包映射为 `flightcore_msgs/Gps` 消息，再传入 Simulink
`GPS_BUS` 边界。

## Actuator 包

方向：WSL2 运行时适配器 -> 仿真器端点。

```json
{
  "protocol_version": 1,
  "packet_type": "actuator",
  "timestamp": 1710000000.0,
  "sequence": 1,
  "mode": "motor",
  "motor_cmd": [0.45, 0.45, 0.45, 0.45]
}
```

字段语义：

| 字段 | 含义 |
|---|---|
| `mode` | `motor` |
| `motor_cmd` | 四个归一化电机指令，FlightCore 顺序，范围 `[0.0, 1.0]` |

端点通过配置将 `motor_cmd` 映射为其原生电机顺序。
不要将此数据包转为 throttle/roll/pitch/yaw 作为主闭环路径。

## Legacy Control 包

方向：WSL2 桥接 -> 仿真器端点。

```json
{
  "protocol_version": 1,
  "packet_type": "control",
  "timestamp": 1710000000.0,
  "sequence": 1,
  "mode": "rate",
  "throttle": 0.45,
  "roll": 0.0,
  "pitch": 0.0,
  "yaw": 0.0
}
```

此数据包保留用于 rate/attitude 调试和模拟测试。它不是 FlightCore 闭环主路径。

本协议不绑定特定仿真器。AirSim、Gazebo、Isaac Sim 或其他后端均需在端点边界处
将其原生 API 适配为本 schema。
