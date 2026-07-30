//
// File: FlightCore_types.h
//
// Code generated for Simulink model 'FlightCore'.
//
// Model version                  : 1.26
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Wed Jul 29 15:51:05 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef FlightCore_types_h_
#define FlightCore_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_StateEstBus_
#define DEFINED_TYPEDEF_FOR_StateEstBus_

// ESKF导航状态估计（位置、速度、姿态、偏置等）
struct StateEstBus
{
  // 位置估计（北东地）
  real32_T Position_NED[3];

  // 速度估计（北东地）
  real32_T Velocity_NED[3];

  // 姿态四元数 [w x y z]
  real32_T Attitude_quat[4];

  // 机体角速度估计
  real32_T AngularRate_Body[3];

  // 机体加速度估计（不含重力）
  real32_T Accel_Body[3];

  // 陀螺零偏估计
  real32_T GyroBias[3];

  // 加速度计零偏估计
  real32_T AccelBias[3];

  // 风速估计（北东地）
  real32_T Wind_NED[3];

  // 估计器状态（0未初始化,1稳定,2错误）
  uint8_T Status;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_FlightCmdBus_
#define DEFINED_TYPEDEF_FOR_FlightCmdBus_

// 飞控统一命令接口（位置、速度、航向、模式、解锁请求和有效标志）
struct FlightCmdBus
{
  // NED位置目标
  real32_T Position_NED_SP[3];

  // NED速度前馈目标
  real32_T Velocity_NED_SP[3];

  // 航向目标
  real32_T Yaw_SP;

  // 命令模式（1=位置保持）
  uint8_T Mode;

  // 解锁请求；不等于实际Armed状态
  boolean_T ArmRequest;

  // 命令有效标志
  boolean_T Valid;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_EscCmdBus_
#define DEFINED_TYPEDEF_FOR_EscCmdBus_

// 飞控至动力系统的电调控制指令
struct EscCmdBus
{
  // 各电机油门指令（归一化）
  real32_T MotorCmd[4];

  // 执行器解锁状态，false时必须输出disarmed安全值
  boolean_T Armed;

  // 当前执行器指令是否有效，false时禁止保持旧命令
  boolean_T Valid;
};

#endif

// Forward declaration for rtModel
typedef struct tag_RTM_FlightCore_T RT_MODEL_FlightCore_T;

#endif                                 // FlightCore_types_h_

//
// File trailer for generated code.
//
// [EOF]
//
