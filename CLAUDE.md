# CLAUDE.md

本文件为 Claude Code 在 UAV 飞控项目中的操作指引，覆盖 Simulink MBD 开发工作流。

## 启动行为

每次对话开始时应先读取本项目下的 `.claude/skills/simulink-mbd.md` 加载 MBD 开发技能。

## 运行规则

### 1. 启动 MATLAB（图形界面模式）

```bash
"D:/Matlab/bin/matlab.exe" -desktop &
```

使用 `-desktop` 参数启动图形界面。`-batch` 模式为无界面批处理，执行完会自动退出。

### 2. 打开 Simulink 工程（自动初始化）

```matlab
openProject('FC_SimulinkProject.prj')
```

工程打开时自动执行 `start.m`，完成以下初始化：
- 设置缓存目录（`.cache/`）
- 调用 `satk_initialize` 建立 MCP 连接

如 MCP 连接失败，手动执行：
```matlab
satk_initialize
```

验证 MCP 连接状态：端口 31515，服务 `matlab-mcp-core-server.exe`。

### 3. 验证初始化

确认以下项目已就绪：
- [ ] MATLAB R2024a+ 运行中
- [ ] MCP 连接正常（端口 31515）
- [ ] 工程已打开，数据字典已加载
- [ ] 工作路径为 `FC_SimulinkProject/`

MCP 工具（model_overview、model_read、model_edit 等）仅在 MATLAB 运行且工程打开后可用。离线模式回退到 `unzip -p` 解析 XML。

## 项目技能

`.claude/skills/` 目录存放开发技能文件：
- `simulink-mbd.md` — Simulink MBD 开发模式

## 常用命令

### MATLAB/Simulink

在 MATLAB（R2025a+）中打开 `FC_SimulinkProject.prj`。`start.m` 在项目打开时自动执行，`shutdown.m` 在关闭时执行。

重建数据字典（YAML 源驱动）：
```matlab
create_GlobalTypes()           % 从 BusConfig/*.m 生成 GlobalTypes.sldd
create_VehicleDict()           % 从 ParamSources/vehicle_params.yaml 生成 VehicleDict.sldd
create_FlightControlDict()     % 从 ParamSources/flight_control_params.yaml
create_PowerSystemDict()       % 从 ParamSources/power_system_params.yaml
create_StateEstDict()          % 从 ParamSources/state_estimation_params.yaml
```

**参数修改流程：** 编辑 `1_Data_Dictionaries/ParamSources/<子系统>_params.yaml` → 运行对应的 `create_*Dict()` → `.sldd` 自动更新。禁止直接手工编辑 `.sldd` 条目。

运行测试：
```matlab
add_test_suites()       % 创建测试套件
test_minimal()          % 最小测试运行器
```

重新配置项目（文件夹注册、快捷方式、标签）：
```matlab
configure_project()
```

### MATLAB/Simulink MCP 服务

项目通过 **Simulink Agentic Toolkit** MCP 服务连接 Claude Code 与 MATLAB。前置条件：MATLAB 运行中，`FC_SimulinkProject.prj` 已打开。如工具调用失败：`satk_initialize`。

| 工具 | 用途 |
|------|------|
| `model_overview` | 查看模型/子系统层级结构、接口和信号连线 |
| `model_read` | 读取块参数、端口、数据类型、MATLAB Function 代码等 |
| `model_edit` | 增删改模型块（add_block、delete_block、set_param） |
| `model_query_params` | 查询模型参数值 |
| `model_resolve_params` | 追踪参数引用链 |
| `model_test` | 运行 Simulink Test 测试用例 |
| `evaluate_matlab_code` | 在 MATLAB 工作区执行代码、运行仿真 |
| `check_matlab_code` | 对 .m 文件进行静态代码分析 |

离线模式（MATLAB 未运行）：`.slx` 是 ZIP 压缩包，`unzip -p <model>.slx simulink/blockdiagram.xml`。

## 架构指针

详细架构文档在 `docs/contracts/`：
- `interface_contract.md` — FlightCore 边界定义（SensorInput/CommandInput/ActuatorOutput/Telemetry）
- `flightcore_runtime_isolation.md` — Runtime Isolation 契约 + DDS 跨界例外条款

路线图与设计思想在 `docs/vision/`。

## 项目强制约束

1. 所有子系统参数通过数据字典管理，不在模型块中硬编码数值
2. Bus 定义仅在 `1_Data_Dictionaries/BusConfig/` 中维护
3. 不将 `slprj/`、`derived/` 纳入版本控制
4. 编写新工具函数前先检查 `5_Tool/` 下已有函数
5. MCP 工具优先于手动 XML 解析；仅在 MATLAB 未运行时回退到 `unzip -p`
6. FlightCore 不得出现仿真器/ROS2/DDS API 符号
7. 契约变更必须先在 `4_Test/` 加测试
