# Windows Simulator Endpoint

本目录仅保留 Windows 侧 AirSim UDP endpoint 及其最小 vendored schema。

**架构权威在 WSL 侧：** UDP protocol、schemas、ROS2 bridge 代码全部在 WSL 原生 workspace
(`~/uavsingle_ros2_ws/src/aircraft_udp_bridge/`) 维护。

## 文件清单

| 文件 | 用途 |
|------|------|
| `windows/airsim_udp_endpoint.py` | AirSim ↔ WSL2 UDP 端点 |
| `schemas/state.schema.json` | vendored from WSL (state packet) |
| `schemas/sensor_imu.schema.json` | vendored from WSL (sensor_imu packet) |
| `schemas/sensor_gps.schema.json` | vendored from WSL (sensor_gps packet) |
| `schemas/actuator.schema.json` | vendored from WSL (actuator packet) |
| `protocol.md` | UDP JSON protocol（与 WSL 同步） |
| `tests/test_protocol.py` | test |
| `tests/test_runtime_adapter_mapping.py` | test |

**Schema 同步规则：** 权威在 WSL `aircraft_udp_bridge/schemas/`。本目录只存放当前 endpoint
实际实现的 4 个 schema (state, sensor_imu, sensor_gps, actuator)。新增/修改 schema 必须
在 WSL 侧进行。

## Windows Endpoint 用法

```powershell
python .\windows\airsim_udp_endpoint.py --bridge-host <WSL2_IP> --rate-hz 50
```

Common options:

```powershell
python .\windows\airsim_udp_endpoint.py `
  --bridge-host <WSL2_IP> `
  --state-port 56000 `
  --control-port 56001 `
  --vehicle-name Drone1 `
  --enable-api-control `
  --arm `
  --motor-order 0,1,2,3
```

`--motor-order` maps endpoint motor API argument order to FlightCore
`motor_cmd` indexes. The default `0,1,2,3` is only a neutral pass-through.

If the endpoint client exposes `moveByMotorPWMsAsync`, actuator packets are sent
through that API. If the API is unavailable, the endpoint logs an unsupported
fallback warning and keeps running. Mock mode logs received actuator packets:

```powershell
python .\windows\airsim_udp_endpoint.py --bridge-host <WSL2_IP> --mock --duration-sec 5
```

## 端口

| 流 | 默认端口 |
|---|---|
| Simulator endpoint → WSL2 状态/观测 | UDP 56000 |
| WSL2 → Simulator endpoint 执行器 | UDP 56001 |

## 完整链路参考

完整架构链路、ROS2 topic 映射、WSL 构建步骤见：
- WSL: `~/uavsingle_ros2_ws/src/aircraft_udp_bridge/README.md`
- WSL: `~/uavsingle_ros2_ws/src/flightcore_runtime_adapter/docs/`
- Contract: `docs/contracts/flightcore_runtime_isolation.md`（含 DDS 跨界例外条款）
