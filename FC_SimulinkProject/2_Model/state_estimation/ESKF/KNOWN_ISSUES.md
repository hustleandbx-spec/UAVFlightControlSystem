# ESKF 模型已知问题

## 问题：MATLAB Function 输出尺寸无法推断

**状态：** 未解决

**影响：** ESKF 模型无法编译，闭环仿真无法运行

**错误信息：**
```
由于模块体中的错误或基础分析的限制，Simulink 无法确定模块 'ESKF/MATLAB Function' 的输出大小和/或类型。
```

## 根因分析

### 1. 外部函数阻断尺寸推断链

ESKF 的 predict/update 函数调用了 `5_Tool/` 下的工具函数：
- `quat_exp` — 四元数指数映射
- `quat_mult` — 四元数乘法
- `quat_normalize` — 四元数归一化
- `quat2rotm` — 四元数转旋转矩阵
- `skew_sym` — 反对称矩阵

这些函数虽然已加 `%#codegen`，但内部使用 `norm`、除法等操作，code generation 无法静态推断返回值尺寸，导致上游 MATLAB Function 的输出尺寸也无法确定。

### 2. 矩阵运算的尺寸推断限制

函数内部大量使用：
```matlab
F_c = zeros(15, 15);
H = zeros(6, 15);
Q_c = diag([...]);
K = (P_p * H') / S;
```

code generation 需要在编译期确定所有变量尺寸，但这些调用的尺寸推断依赖上下文中的外部函数返回值。

### 3. Stateflow Data 显式设置无效

即使通过 Stateflow API 显式设置了输出端口的 `DataType` 和 `Props.Array.Size`，MATLAB Function 块仍然无法从函数体内部推断类型，报错不变。

## 已尝试的修复（均未成功）

| 方案 | 结果 |
|------|------|
| 显式设置 Stateflow Data 的 Size='16' 和 DataType='single' | 无效 |
| 添加 `coder.varsize` 声明 | 无效 |
| 移除 `coder.varsize`，改用 `zeros(16,1,'single')` 初始化 | 无效 |
| 工具函数加 `%#codegen` | 无效 |
| 工具函数中 `norm` 替换为 `sqrt` 手动实现 | 无效 |

## 建议解决方向

### 方案 A：重写为 Simulink 标准块（推荐）

将 predict/update 逻辑拆解为 Simulink 标准块：
- 矩阵运算 → Gain / Product / Sum 块
- 四元数运算 → 封装为独立子系统
- 协方差传播 → 矩阵乘法块

**优点：** 可读性好，code generation 兼容，与项目 MBD 理念一致
**缺点：** 工作量大，需要重构整个 ESKF 模型

### 方案 B：合并为单个 MATLAB Function + coder.extrinsic

将所有外部函数调用声明为 `coder.extrinsic`，让 code generation 跳过这些函数的内联分析：

```matlab
coder.extrinsic('quat_exp', 'quat_mult', 'quat_normalize', 'quat2rotm', 'skew_sym');
```

**优点：** 改动小
**缺点：** `coder.extrinsic` 函数在代码生成时会被替换为空操作，需要额外处理

### 方案 C：将工具函数内联到 MATLAB Function

将 `quat_exp`、`quat_mult` 等函数的实现直接写入 predict/update 函数体中，消除外部调用。

**优点：** 消除推断链断裂
**缺点：** 代码重复，维护困难

## 相关文件

- `2_Model/state_estimation/ESKF/ESKF.slx` — ESKF 模型
- `2_Model/state_estimation/ESKF/ESKF.slx` — Subsystem/MATLAB Function1（更新步）
- `5_Tool/quat_exp.m` — 四元数指数映射
- `5_Tool/quat_mult.m` — 四元数乘法
- `5_Tool/quat_normalize.m` — 四元数归一化
- `5_Tool/quat2rotm.m` — 四元数转旋转矩阵
- `5_Tool/skew_sym.m` — 反对称矩阵
