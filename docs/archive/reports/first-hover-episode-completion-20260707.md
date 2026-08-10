# 第一个悬停 Episode 修正报告

> 日期：2026-07-07  
> 范围：修正 claude-fable-5 原 P0-P5 完成报告  
> 结论：P0 基本完成；P1 有局部 smoke 资产但未完成端到端验收；P2-P5 未完成。

---

## 一句话结论

原报告把“脚本就绪 / 基建已创建”多处写成“阶段验收完成”。按 `DEVELOPMENT_PLAN.md` 的判据，真实 AirSim 外部闭环和 30 s 可回放 episode 尚未跑通，不能把 P2-P5 标为完成。

---

## 阶段状态

| 阶段 | 当前判定 | 依据 |
|---|---|---|
| P0 文档与边界收敛 | 基本完成 | README、DEVELOPMENT_PLAN、docs/contracts 已形成当前主线与冻结面 |
| P1 Mock 端到端 smoke | 部分完成 | MATLAB 内部 smoke 与 Windows 协议测试存在；但 WSL bridge + adapter + MATLAB + endpoint 的同一 episode 目录证据尚未形成 |
| P2 AirSim motor API 探针 | 未完成 | `probe_airsim_motors.py` 只是脚本；尚无真实 AirSim API 调用、电机响应和 `--motor-order` 结论 |
| P3 真实 AirSim 外部闭环 | 未完成 | AirSim -> WSL -> MATLAB -> AirSim actuator 未形成真实闭环证据 |
| P4 第一个悬停 episode | 未完成 | 尚无 30 s rosbag2 / manifest / logs / clock_offsets / metrics / plots 的完整可拷走目录 |
| P5 V0 数据地板固化 | 未完成 | sequence、health、IsNew、时钟容差和 manifest schema 尚未由真实 episode 反写为契约/测试 |

---

## 本轮修复

| 文件 | 修复内容 |
|---|---|
| `bridge/airsim_ros2_udp_bridge/tests/test_mock_smoke.py` | 修正 vendored `protocol.py` import 路径 |
| `bridge/airsim_ros2_udp_bridge/tests/test_protocol.py` | Windows 侧协议测试改用 vendored `windows/protocol.py`，不再引用已迁出 WSL 的包 |
| `bridge/airsim_ros2_udp_bridge/tests/test_runtime_adapter_mapping.py` | WSL-only adapter mapping 测试在 Windows 仓库中显式 skip，避免伪失败 |
| `bridge/airsim_ros2_udp_bridge/windows/airsim_udp_endpoint.py` | actuator 去重改为 `sequence + timestamp`，避免当前 `sequence=0` 时只执行第一条命令；真实 AirSim actuator 也写日志 |
| `scripts/run_hover_episode.sh` | 自动检测/强制传入 Windows endpoint host，并传给 `flightcore_runtime_adapter.actuator_target_host` |
| `scripts/run_windows_endpoint.py` | 补齐 `--motor-order`、AirSim host/port、vehicle name 透传 |
| `scripts/generate_manifest.py` | 支持 launcher 注入 WSL commit、mode、duration |
| `scripts/evaluate_episode.py` | 改用 `run_summary.json` 的实际时长；识别 rosbag2 `metadata.yaml/.db3/.mcap`；mock/airsim 分模式判据 |
| `scripts/run_matlab_flightcore_ros2_episode.m` | 新增 MATLAB 侧 episode runner |
| `scripts/test_hover_episode.ps1` | 新增一键悬停 episode 测试编排脚本 |
| 用户级 MCP/Codex 配置 | `.claude.json` 与 Codex 旧 rule 中的 `D:\Maltab` 已改为 `D:\MATLAB\R2025b`，并移除 `D:\Maltab` junction |

---

## 一键测试入口

主交付物：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\test_hover_episode.ps1 -Mode mock -DurationSec 30
```

默认行为：

1. 创建统一 episode 目录：`episodes/YYYYMMDD_HHMMSS_airsim_hover_v0/`
2. 运行 Windows 协议/endpoint 单元测试。
3. 生成 `manifest.yaml`。
4. 启动 WSL `aircraft_udp_bridge` + `flightcore_runtime_adapter` + rosbag2 + clock recorder。
5. 启动 MATLAB `FlightCore_ROS2_loop`。
6. 启动 Windows endpoint。
7. 写入 `run_summary.json`。
8. 运行 `evaluate_episode.py` 生成 `metrics.json`。

Windows-only 启动器验证：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\test_hover_episode.ps1 `
  -SkipWsl -SkipMatlab -NoEvaluate -SkipUnitTests -DurationSec 1
```

真实 AirSim 模式：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\test_hover_episode.ps1 `
  -Mode airsim -DurationSec 30 -MotorOrder 0,1,2,3
```

`Mode=mock` 只验证链路 smoke，不代表 P4 真实悬停 episode。`Mode=airsim` 才按完整 episode 判据要求 rosbag、日志、clock_offsets、actuator feedback、plots 和飞行表现。

---

## 仍未解决

| 问题 | 影响 | 下一步 |
|---|---|---|
| FlightCore 输出 `sequence=0` | 已由 endpoint 用 timestamp 缓解，但 P5 数据地板仍不合格 | 在 Simulink Bus Assignment/Bus Creator 层填充 sequence |
| WSL `bridge_node.py` 改动未固化到 WSL git commit | episode manifest 无法指向可复现 WSL 源码状态 | 在 WSL 原生 repo 中提交或形成同步流程 |
| `/uav/health/status` publisher 未实测 | clock recorder 依赖该 topic，真实三时钟域记录仍可能为空 | 补 SystemHealth 发布或调整 clock 记录输入源 |
| AirSim motor order 未探针 | 真实执行器级闭环方向可能错误 | 运行 `scripts/probe_airsim_motors.py` 并写入 `--motor-order` |
| 控制器饱和 `[1,1,1,1]` | 即使闭环打通也不能说明悬停能力 | P2 后调参或导入四旋翼测试床参数 |

---

## 验证记录

本轮已验证：

```text
python -m unittest discover bridge\airsim_ros2_udp_bridge\tests
-> Ran 21 tests OK, skipped=1

python -m py_compile scripts\generate_manifest.py scripts\evaluate_episode.py scripts\run_windows_endpoint.py bridge\airsim_ros2_udp_bridge\windows\airsim_udp_endpoint.py bridge\airsim_ros2_udp_bridge\windows\protocol.py
-> OK

powershell -ExecutionPolicy Bypass -File .\scripts\test_hover_episode.ps1 -SkipWsl -SkipMatlab -NoEvaluate -SkipUnitTests -DurationSec 1
-> Windows endpoint launcher PASS
```

最小 launcher 验证不代表 P1/P4 完成；它只证明新增一键脚本的 Windows 编排和 endpoint 退出路径可运行。
