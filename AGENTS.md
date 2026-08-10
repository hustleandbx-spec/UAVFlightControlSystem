# AGENTS.md

操作指引统一在 `CLAUDE.md` 中维护。本文件仅保留 Codex 特定的差异配置。

## Codex 启动差异

### 启动 MATLAB（Windows PowerShell）

```powershell
Start-Process -FilePath 'D:\MATLAB\R2025b\bin\matlab.exe' `
  -ArgumentList '-desktop' `
  -WorkingDirectory 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject'
```

### MCP 连接

```matlab
addpath(fullfile(getenv('USERPROFILE'), ...
    '.matlab', 'agentic-toolkits', 'simulink'));
satk_initialize
validate_installation
```

`validate_installation` 应在 `satk_initialize` 之后执行。连接器端口是动态值，不以固定端口号作为健康判据。

### 验证条件

- [ ] MATLAB R2025b 运行中
- [ ] `validate_installation` 显示 `Result: PASS`
- [ ] MCP 能成功调用 `model_overview`
- [ ] 工程已打开，数据字典已加载
- [ ] 工作路径为 `FC_SimulinkProject/`

### 可用 MCP 工具

当前 Simulink Agentic Toolkit 注册的六个模型工具：`model_overview`、`model_read`、`model_edit`、`model_query_params`、`model_resolve_params`、`model_test`。

`evaluate_matlab_code`、`check_matlab_code` 属于 MATLAB MCP Core Server 能力，不在上述六工具中；仅在当前 MCP 客户端实际暴露时使用。

## 架构约束（与 CLAUDE.md 共享）

架构指针见 `docs/archive/contracts/`；路线图见 `docs/vision/`。项目强制约束见 CLAUDE.md "项目强制约束" 节，此处不重复。
