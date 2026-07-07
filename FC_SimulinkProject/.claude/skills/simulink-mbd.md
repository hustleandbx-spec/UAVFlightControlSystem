---
name: simulink-mbd
description: Simulink Model-Based Design development patterns for the UAV flight control project. Covers .slx file inspection, data dictionary management, bus configuration, MATLAB Function blocks (Stateflow EML), testing, and engineering standards.
---

# Simulink MBD 开发技能 — UAV 飞控项目

## 项目架构原则

本项目以适航（ARP4754A/DO-331）为工程目标，遵循以下架构约束：

1. **数据字典分离**：每个子系统有独立字典（`XxxDict.sldd`），仅存本模块独有参数
2. **全局总线字典**：`1_Data_Dictionaries/GlobalTypes.sldd` 定义所有总线，各子系统通过 `addDataSource` 引用
3. **飞行器总体字典**：`1_Data_Dictionaries/VehicleDict.sldd` 存放质量、惯量、几何参数等跨模块共用参数（待建立）
4. **每个子系统一个测试套件**：顶层 `4_Test/` 为系统级，各子系统目录内含 Harness
5. **System Composer 顶层架构**：`3_Simluation/UAV_FC.slx` 描述系统集成

## 项目目录结构

```
FC_SimulinkProject/
├── FC_SimulinkProject.prj          # Simulink Project 文件
├── start.m                         # 环境初始化（需增强）
├── CLAUDE.md                       # Claude 行为规则
├── 1_Data_Dictionaries/            # 全局类型与总体参数
│   ├── create_GlobalTypes.m        # 扫描 BusConfig/ 生成 GlobalTypes.sldd
│   ├── GlobalTypes.sldd            # 所有总线类型定义
│   ├── VehicleDict.sldd            # 飞行器总体参数（待建立）
│   └── BusConfig/
│       ├── config_DynamicModelBus.m
│       ├── config_EscCmdBus.m
│       ├── config_StateEstBus.m
│       └── config_ThrustTorqueBus.m
├── 2_Model/                        # 子系统模型
│   ├── control/                    # 飞控子系统
│   │   ├── UAV_FlightControl.slx
│   │   ├── FlightControlDict.sldd
│   │   └── create_FlightControlDict.m
│   ├── dynamic_model/              # 六自由度动力学
│   │   ├── UAV_dynamic_model.slx
│   │   ├── DynamicModelDict.sldd
│   │   └── create_DynamicModelDict.m
│   ├── power_systerm/              # 动力系统（注意拼写）
│   │   ├── power_systerm_model.slx
│   │   ├── PowerSystemDict.sldd
│   │   └── create_PowerSystemDict.m
│   ├── state_estimation/           # 导航估计
│   │   └── ESKF.slx
│   └── sensor_model/               # 传感器模型
│       ├── sensor_GPS_model.slx
│       └── sensor_IMU_model.slx
├── 3_Simluation/                   # 仿真集成（注意拼写）
│   ├── UAV_FC.slx                  # 顶层 System Composer 架构
│   └── dynamic_model.slx           # 动力+动力学开环仿真
├── 4_Test/                         # 测试
│   └── UAV_FlightControlSystem_Test_Case.mldatx
├── 5_Tool/                         # 工具函数
│   ├── addBusFromCell.m
│   ├── addVarsFromCell.m
│   └── checkAndCloseDictionary.m
├── resources/project/              # Simulink Project 元数据
├── derived/                        # 构建产物（应 gitignore）
└── slprj/                          # 仿真缓存（应 gitignore，分散多处）
```

## Simulink 模型文件（.slx）操作

**核心原理：** `.slx` 文件是 ZIP 压缩包，包含 XML 文件描述模型结构和内容。

### 读取模型元数据
```
unzip -p model.slx simulink/blockdiagram.xml   # 模型信息、字典引用、UUID
```

### 读取顶层系统结构（模块连线关系）
```
unzip -p model.slx simulink/systems/system_root.xml
```

### 读取子系统内部结构
```
unzip -p model.slx simulink/systems/system_N.xml    # N 是 system_root 中 System Ref 的编号
```

### 读取 MATLAB Function 代码（关键）
MATLAB Function 块在 Simulink 中**不是**直接放在 `<System>` XML 里的。它通过 Stateflow S-Function (`sf_sfun`) 实现，实际代码在两个地方：

1. **Stateflow Machine 索引** — `simulink/stateflow/machine.xml` 提供 chart 引用关系
2. **Chart 文件** — `simulink/stateflow/chart_N.xml` 包含实际 MATLAB 代码

查找流程：
```
① 在 system_N.xml 中找到 S-Function 块，记录其 Name（如 "MATLAB Function"）
② 读取 simulink/stateflow/machine.xml 获取 chart id 映射
   <instance name="子系统名/MATLAB Function" chart="15"/>
③ 读取 simulink/stateflow/chart_15.xml
④ 代码在 <P Name="script">...</P> 标签内
```

### 读取 System Composer 架构
```
unzip -p model.slx simulink/systemcomposer/architecture.xml   # 含组件、接口、连线
```

### 查看模型引用关系
```
unzip -p model.slx simulink/modelDictionary.xml
```
如果模型引用了数据字典，`blockdiagram.xml` 中会有 `<P Name="DataDictionary">XxxDict.sldd</P>`。

## 数据字典操作模式

### 模式：创建全局类型字典
```matlab
% 1. 定义配置文件 config_XxxBus.m，返回包含 busName 和 elements 的结构体
% 2. 运行 create_GlobalTypes() 自动扫描 BusConfig/ 目录
% 3. elements 格式：{Name, DataType, Dimensions, Unit, Description}
```

### 模式：创建子系统字典
```matlab
% 1. 调用 checkAndCloseDictionary 安全关闭/删除旧字典
% 2. Simulink.data.dictionary.create(dictName)
% 3. ddObj.addDataSource('GlobalTypes.sldd')  引用总线
% 4. addVarsFromCell(dData, VarTable)          批量添加参数
% 5. VarTable 格式：{Name, Value, DataType, StorageClass, Description}
```

### 存储类选型指导
- `ExportedGlobal` — 需要跨模块访问或标定的参数（如 PID 增益）
- 纯常数（如 g=9.8）— 考虑直接用 Constant 块或 `#define` 级参数
- 内部变量 — 不要过度暴露为全局

## MATLAB Function 编写规范

### 函数签名模式
```matlab
function [out1, out2] = fcn_name(in1, in2, in3)
```
输入输出在 `chart_N.xml` 的 `<data>` 标签中声明，数据类型通常为 `Inherit: Same as Simulink`。

### 已实现的 MATLAB Function 清单

| 所属模型 | Chart | 函数名 | 功能 |
|---------|-------|--------|------|
| ESKF | chart_11 | `fcn_predict_eskf` | ESKF 预测步（IMU积分+协方差传播） |
| ESKF | chart_22 | `fcn_update` | ESKF GPS更新（Joseph形式） |
| DynamicModel | chart_8 | `fcn` | 四元数微分 `q_dot=0.5*Omega*q` |
| DynamicModel | chart_16 | `quat2rotm` | 四元数→旋转矩阵 |
| DynamicModel | chart_23 | `fcn_quat_normalize` | 四元数归一化 |
| PowerSystem | chart_15 | `fcn_calc_wss` | 油门→稳态转速 |
| PowerSystem | chart_25 | `fcn_prop_forces` | 转速→单电机推力/扭矩 |
| PowerSystem | chart_37 | `fcn_geometry_mixer` | X型四旋翼几何混控 |

### 开发新 MATLAB Function 时的检查点
- 不要重复实现已有工具函数（如 `quat2rotm`、`quat_mult` 已在多处存在）
- SIMD/代码生成友好：用向量运算代替 for 循环
- 保护性归一化：四元数运算后强制单位化

## 总线配置规范

### 新建总线的步骤
1. 在 `1_Data_Dictionaries/BusConfig/` 下创建 `config_XxxBus.m`
2. 函数返回包含 `busName`、`description`、`elements` 的结构体
3. 运行 `create_GlobalTypes()` 重建字典
4. 需要该总线的子系统字典无需修改（已引用 GlobalTypes）

### 现有总线及其使用者

| 总线 | 生产者 | 消费者 |
|------|--------|--------|
| `DynamicModelBus` | DynamicModel | Sensor, (Controller 待接入) |
| `StateEstBus` | ESKF | FlightControl |
| `EscCmdBus` | FlightControl | PowerSystem |
| `ThrustTorqueBus` | PowerSystem | DynamicModel |

## 测试工程规范

### 测试分层
```
L1 单元测试: MATLAB Function 独立输入/输出验证 (MATLAB Unit Test)
L2 部件测试: 子系统 Harness + Simulink Test (assert 模块)
L3 集成测试: 多子系统串联 (如 3_Simluation/dynamic_model.slx)
L4 系统测试: 完整闭环 + 需求覆盖 (UAV_FC.slx)
```

### Simulink Test 文件
- `.mldatx` 文件本身是 ZIP，包含测试用例、输入数据、评估条件
- Harness 在模型内以 `_Harness_N` 子系统形式存在

### 给测试加断言
不要只依赖 Scope 看波形。在 Harness 中添加：
- `verify` 语句块（Simulink Test Assessment）
- `assert` 模块检查状态范围
- 仿真数据 Inspector 的编程化比较

## 工程管理检查清单

### 编码规范
- [ ] 无 `slprj/` 目录混入源码
- [ ] 字典引用路径健壮（不依赖 cd 到特定目录）
- [ ] 命名一致：`power_systerm` vs `power_system`, `Simluation` vs `Simulation`
- [ ] 新 MATLAB Function 不重复实现已有的工具函数

### 版本控制
- [ ] `.gitignore` 包含 `slprj/`, `derived/`, `*.slxc`, `resources/project/`
- [ ] 只提交手写源码，不提交构建产物

### Simulink Project 配置
- [ ] `start.m` 初始化路径（项目根目录加入 PATH）
- [ ] 文件夹分类：sources / derived 明确区分
- [ ] 启动时自动加载所有数据字典

### 文档
- [ ] 系统级需求清单（至少一页纸）
- [ ] 接口控制文档（总线信号含义与单位）
- [ ] 测试计划（每个子系统被测什么、判据是什么）

## 常见错误与解决

| 症状 | 可能原因 | 解决 |
|------|---------|------|
| 字典引用失败 | 相对路径依赖工作目录 | 用 Simulink Project 路径管理或 start.m 统一 addpath |
| 读不到 MATLAB Function 代码 | 代码不在 subsystem XML 里 | 去 stateflow/chart_N.xml 里找 `<P Name="script">` |
| 模型加载报 bus 类型未定义 | GlobalTypes.sldd 未在路径中 | 确认字典已打开或路径正确 |
| Harness 无法运行 | Harness 的 Model Reference 指向不存在的模型版本 | 检查 ConfigSet 中的 ModelReference 设置 |
