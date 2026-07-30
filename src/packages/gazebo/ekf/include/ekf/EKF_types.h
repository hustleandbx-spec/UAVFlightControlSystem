//
// File: EKF_types.h
//
// Code generated for Simulink model 'EKF'.
//
// Model version                  : 4.30
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Wed Jul 29 15:49:11 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef EKF_types_h_
#define EKF_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_IMU_BUS_
#define DEFINED_TYPEDEF_FOR_IMU_BUS_

// IMU sensor raw measurement frame
struct IMU_BUS
{
  // Body-frame specific force
  real32_T Accel[3];

  // Body-frame angular rate
  real32_T Gyro[3];

  // Frame measurement validity flag
  boolean_T Valid;

  // Measurement sample timestamp
  real_T Timestamp;

  // IMU msg is new
  boolean_T IsNew;
};

#endif

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

// Forward declaration for rtModel
typedef struct tag_RTM_EKF_T RT_MODEL_EKF_T;

#endif                                 // EKF_types_h_

//
// File trailer for generated code.
//
// [EOF]
//
