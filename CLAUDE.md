# CLAUDE.md

本文件为 Claude Code 在 UAV 单机飞控项目中的操作指引，覆盖 MATLAB/Simulink MBD 开发工作流。架构入口见 [README.md](README.md)，当前执行顺序见 [docs/process/05_Development_Plan.md](docs/process/05_Development_Plan.md)。

## 启动行为

每次对话开始时先读取根目录 README、开发计划（`docs/process/05_Development_Plan.md`）和相关契约文档。MBD 技能由 `.claude/settings.json` 启用的插件提供：`model-based-design-core`（v0.6.0，含 `model_check` 用法）、`matlab-core`（v0.4.1）、`matlab-software-development`（v0.5.1），均于 2026-08-12 同步至官方最新（需重启会话生效）。项目内已无 `.claude/skills/` 文件。插件市场源在 `D:\AI\MPCserver\matlabMCP\` 下两个仓库（git 已对齐官方 main，旧版备份 `*.bak-20260812`）。

## 启动 MATLAB

Windows PowerShell：

```powershell
Start-Process -FilePath 'D:\MATLAB\R2025b\bin\matlab.exe' `
  -ArgumentList '-desktop' `
  -WorkingDirectory 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject'
```

打开工程：

```matlab
openProject('D:\Project\UAVSingleFlightControl\FC_SimulinkProject\FC_SimulinkProject.prj')
```

工程打开后会执行 `start.m`。如 MCP 未初始化，手动执行：

```matlab
addpath(fullfile(getenv('USERPROFILE'), ...
    '.matlab', 'agentic-toolkits', 'simulink'));
satk_initialize
validate_installation
```

`validate_installation` 应在 `satk_initialize` 后执行。连接器端口是动态值，不以固定端口号作为健康判据。

## 验证条件

- MATLAB R2025b 运行中。
- `validate_installation` 显示 `Result: PASS`。
- MCP 能成功调用 `model_overview`。
- 工程已打开，数据字典已加载。
- 工作路径为 `FC_SimulinkProject/`。
- MCP 注册的扩展文件必须指向 `C:/Users/89447/.matlab/agentic-toolkits/simulink/tools/tools.json`（旧路径 `D:/个人/matlabMCP/...` 已失效），可用 `claude mcp list` 检查。

当前本地安装注册的八个模型工具（2026-08-12 同步自官方 main；旧版 SATK-v2026.05.21 已备份于 `~/.matlab/agentic-toolkits/simulink.bak-20260812`）：

| 工具 | 用途 |
|---|---|
| `model_overview` | 查看模型/子系统层级结构、接口和信号连线 |
| `model_read` | 读取块拓扑、信号连线与算法表达式（`@Param(变量)` 引用用 `model_resolve_params` 解析数值） |
| `model_edit` | 增删改模型块和参数 |
| `model_query_params` | 查询块/信号/模型配置参数值 |
| `model_resolve_params` | 将 `model_read` 中的变量引用解析为工作区/数据字典中的数值并定位来源 |
| `model_check` | 结构健康检查：未连接端口、悬空信号、Stateflow edit-time lint |
| `model_read_diagnostics` | 读取 Diagnostic Viewer 的诊断消息（error/warning/info） |
| `model_test` | 运行 Gherkin 格式行为测试（先加载 `testing-simulink-models` 技能） |

`evaluate_matlab_code`、`check_matlab_code` 属于 MATLAB MCP Core Server 能力，仅在当前 MCP 客户端实际暴露时使用。

## 常用命令

重建数据字典：

```matlab
create_GlobalTypes()
create_VehicleDict()
create_FlightControlDict()
create_PowerSystemDict()
create_StateEstDict()
```

参数修改流程：

```text
编辑 1_Data_Dictionaries/ParamSources/<subsystem>_params.yaml
  -> 运行对应 create_*Dict()
  -> .sldd 自动更新
```

禁止直接手工编辑 `.sldd` 条目。

运行测试：

```matlab
add_test_suites()
test_minimal()
```

重新配置工程：

```matlab
configure_project()
```

## 架构指针

| 内容 | 位置 |
|---|---|
| 项目入口和文档体系 | `README.md` |
| 当前开发计划 | `docs/process/05_Development_Plan.md` |
| 路线图与消息思想 | `docs/vision/` |
| InterfaceContract 与 Runtime Isolation（历史） | `docs/archive/contracts/` |
| ROS2 topic 与 Simulink Bus 映射 | `FC_SimulinkProject/3_Integration/ROS2/` |
| Windows AirSim endpoint | `bridge/airsim_ros2_udp_bridge/` |

## 项目强制约束

0. 使用任何 MATLAB MCP 工具前，必须先打开 `FC_SimulinkProject` 工程（`FC_SimulinkProject.prj`），确认工程初始化完成后再调用 MCP；不得在未打开工程时直接连接或读取模型。
1. 所有子系统参数通过数据字典管理，不在模型块中硬编码数值。
2. Bus 定义仅在 `1_Data_Dictionaries/BusConfig/` 中维护。
3. 不将 `slprj/`、`derived/`、`build/`、`install/`、`log/` 纳入版本控制。
4. 编写新工具函数前先检查 `5_Tool/` 下已有函数。
5. MCP 工具优先于手动 XML 解析；仅在 MATLAB 未运行时回退到 `.slx` ZIP/XML 检查。
6. FlightCore 不得出现仿真器、ROS2、DDS、UDP、PlotJuggler、rosbag2 API 或符号。
7. `/aircraft/*`、truth、可视化和日志数据不得作为 FlightCore 控制闭环输入。
8. 契约变更必须先在 `4_Test/` 加测试，再改模型、adapter、bridge 或部署代码。
9. Windows 侧文件在 Windows 仓库维护；WSL ROS2 源码在 WSL 原生 filesystem 维护。

