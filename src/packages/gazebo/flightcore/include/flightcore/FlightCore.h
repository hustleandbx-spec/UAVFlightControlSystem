//
// File: FlightCore.h
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
#ifndef FlightCore_h_
#define FlightCore_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "FlightCore_types.h"
#include "EKF.h"
#include "UAV_FlightControl.h"
#include "rt_zcfcn.h"
#include "model_reference_types.h"
#include "zero_crossing_types.h"

// Block signals for model 'FlightCore'
struct B_FlightCore_c_T {
  StateEstBus BusConversion_InsertedFor_UAV_F;
  FlightCmdBus FlightCmdBus_Creator;   // '<S1>/FlightCmdBus_Creator'
  EscCmdBus EscCmdBus_g;               // '<Root>/UAV_FlightControl'
  int32_T y;
  boolean_T ArmRequest;                // '<S1>/Data Type Conversion'
  boolean_T Compare;                   // '<S4>/Compare'
  boolean_T ControlActive;             // '<S2>/Chart'
  boolean_T Armed;                     // '<S2>/Chart'
  boolean_T Ready;
  boolean_T b;
};

// Block states (default storage) for model 'FlightCore'
struct DW_FlightCore_f_T {
  uint8_T is_active_c3_FlightCore;     // '<S2>/Chart'
  uint8_T is_c3_FlightCore;            // '<S2>/Chart'
  uint8_T previousZC;                  // '<S2>/Chart'
  MdlrefDW_EKF_T EKF_InstanceData;     // '<Root>/EKF'
  MdlrefDW_UAV_FlightControl_T UAV_FlightControl_InstanceData;// '<Root>/UAV_FlightControl' 
};

// Zero-crossing (trigger) state for model 'FlightCore'
struct ZCE_FlightCore_T {
  ZCSigState SFunction_edgeDetectionSignal_Z;// '<S2>/Chart'
};

// Real-time Model Data Structure
struct tag_RTM_FlightCore_T {
  const char_T **errorStatus;
  RTWSolverInfo *solverInfo;
  const rtTimingBridge *timingBridge;

  //
  //  Timing:
  //  The following substructure contains information regarding
  //  the timing information for the model.

  struct {
    time_T stepSize0;
    int_T mdlref_GlobalTID[4];
    SimTimeStep *simTimeStep;
  } Timing;

  time_T getClockTickH0() const;
  time_T getClockTick0() const;
  time_T getClockTickH1() const;
  time_T getClockTick1() const;
  time_T getClockTickH2() const;
  time_T getClockTick2() const;
  time_T getClockTickH3() const;
  time_T getClockTick3() const;
  const char_T* getErrorStatus() const;
  void setErrorStatus(const char_T* const aErrorStatus) const;
  const char_T** getErrorStatusPointer() const;
  void setErrorStatusPointer(const char_T** aErrorStatusPointer);
  boolean_T isMajorTimeStep() const;
  boolean_T isMinorTimeStep() const;
  boolean_T isSampleHit(int32_T sti) const;
  SimTimeStep getSimTimeStep() const;
  SimTimeStep* getSimTimeStepPointer() const;
  void setSimTimeStepPointer(SimTimeStep* aSimTimeStepPointer);
  time_T getT() const;
};

struct MdlrefDW_FlightCore_T {
  B_FlightCore_c_T rtb;
  DW_FlightCore_f_T rtdw;
  RT_MODEL_FlightCore_T rtm;
  ZCE_FlightCore_T rtzce;
};

//
//  Exported Global Parameters
//
//  Note: Exported global parameters are tunable parameters with an exported
//  global storage class designation.  Code generation will declare the memory for
//  these parameters and exports their symbols.
//

extern real32_T R_pos;                 // Variable: R_pos
                                          //  Referenced by: '<Root>/EKF'
                                          //  GPS 位置观测噪声标准差。增大→降低 GPS 位置在修正中的权重

extern real32_T R_vel;                 // Variable: R_vel
                                          //  Referenced by: '<Root>/EKF'
                                          //  GPS 速度观测噪声标准差。增大→降低 GPS 速度在修正中的权重

extern real32_T SE_EKF_INIT_STATE[16]; // Variable: SE_EKF_INIT_STATE
                                          //  Referenced by: '<Root>/EKF'
                                          //  EKF 16维默认名义状态 [Position_NED; Velocity_NED; Attitude_quat_wxyz; AccelBias; GyroBias]。位置以启动点为局部 NED 原点。

extern real32_T attP;                  // Variable: attP
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  姿态 P 增益。姿态误差→角速率指令。只用P，I由角速率环承担，防止两级积分器串联振荡

extern real32_T ekf_predict_dt;        // Variable: ekf_predict_dt
                                          //  Referenced by: '<Root>/EKF'
                                          //  EKF 预测步长数值输入。显式匹配 EKF MATLAB Function 的 single 数值契约

extern real32_T g_n[3];                // Variable: g_n
                                          //  Referenced by: '<Root>/EKF'
                                          //  NED 重力加速度矢量 [g_N, g_E, g_D]。物理常量，不作调参项

extern real32_T mass;                  // Variable: mass
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  AirSim Generic F450 四旋翼总质量（含电池、载荷）。来源: MultiRotorParams::setupFrameGenericQuad

extern real32_T maxAcc_xy;             // Variable: maxAcc_xy
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  水平加速度指令上限。限制最大姿态倾斜角

extern real32_T maxAcc_z;              // Variable: maxAcc_z
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  垂直加速度指令上限

extern real32_T maxRate;               // Variable: maxRate
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  最大角速率指令 (= π rad/s ≈ 180°/s)

extern real32_T maxTorque;             // Variable: maxTorque
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  单轴力矩指令上限。在电机饱和前拦截过大指令，确保控制在线性区

extern real32_T maxVel_xy;             // Variable: maxVel_xy
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  水平速度指令上限

extern real32_T maxVel_z;              // Variable: maxVel_z
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  垂直速度指令上限

extern real32_T mixMatrix[16];         // Variable: mixMatrix
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  X型四旋翼混控矩阵 (4×4)。[F, τx, τy, τz]&#x1D40; → [m1, m2, m3, m4]&#x1D40;。列分别对应: 总推力、滚转力矩、俯仰力矩、偏航力矩

extern real32_T motorArmLength_m;      // Variable: motorArmLength_m
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  AirSim Generic F450 电机到重心的水平距离（臂长）

extern real32_T motorMax;              // Variable: motorMax
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  电机指令最大值（归一化 0~1）。硬件保护上限

extern real32_T motorMaxReactionTorque_Nm;// Variable: motorMaxReactionTorque_Nm
                                             //  Referenced by: '<Root>/UAV_FlightControl'
                                             //  AirSim Generic Quad 单旋翼最大反扭矩。normalized command 1.0 线性对应此反扭矩

extern real32_T motorMaxThrust_N;      // Variable: motorMaxThrust_N
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  AirSim Generic Quad 单旋翼最大推力。normalized command 1.0 线性对应此推力

extern real32_T motorMin;              // Variable: motorMin
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  电机指令最小值（归一化 0~1）。>0 防止电机停转失去姿态调控能力

extern real32_T posD_xy;               // Variable: posD_xy
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  水平位置 D 增益。当前=0（微分由角速率环 D 提供）

extern real32_T posD_z;                // Variable: posD_z
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  高度位置 D 增益。当前=0

extern real32_T posFilterN;            // Variable: posFilterN
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  位置PID导数滤波系数；在50 Hz周期下满足N*Ts不超过0.5。

extern real32_T posI_xy;               // Variable: posI_xy
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  水平位置 I 增益。消除悬停稳态位置误差和常值风扰

extern real32_T posI_z;                // Variable: posI_z
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  高度位置 I 增益

extern real32_T posP_xy;               // Variable: posP_xy
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  水平位置 P 增益。位置误差→速度指令。增大加快位置响应，过大引发超调

extern real32_T posP_z;                // Variable: posP_z
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  高度位置 P 增益。垂直增益 > 水平增益以积极对抗重力偏差

extern real32_T rateD;                 // Variable: rateD
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  角速率 D 增益。提供角速率阻尼，抑制姿态修正时的振荡

extern real32_T rateFilterN;           // Variable: rateFilterN
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  角速率PID导数滤波器系数。

extern real32_T rateI;                 // Variable: rateI
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  角速率 I 增益。消除角速率稳态误差、补偿常值扰动力矩

extern real32_T rateP;                 // Variable: rateP
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  角速率 P 增益。对抗角速率扰动（突风、重心偏移）

extern real32_T sigma_acc;             // Variable: sigma_acc
                                          //  Referenced by: '<Root>/EKF'
                                          //  加速度计白噪声 PSD。增大→降低加速度计在融合中的权重

extern real32_T sigma_ba;              // Variable: sigma_ba
                                          //  Referenced by: '<Root>/EKF'
                                          //  加速度计偏置随机游走 PSD。增大→允许偏置估计更快变化但噪声更大

extern real32_T sigma_bg;              // Variable: sigma_bg
                                          //  Referenced by: '<Root>/EKF'
                                          //  陀螺仪偏置随机游走 PSD。增大→允许偏置估计更快变化但噪声更大

extern real32_T sigma_gyr;             // Variable: sigma_gyr
                                          //  Referenced by: '<Root>/EKF'
                                          //  陀螺仪白噪声 PSD。增大→降低陀螺仪在融合中的权重

extern real32_T velD_xy;               // Variable: velD_xy
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  水平速度 D 增益。当前=0

extern real32_T velD_z;                // Variable: velD_z
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  垂直速度 D 增益。当前=0

extern real32_T velFilterN;            // Variable: velFilterN
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  速度PID导数滤波系数；在50 Hz周期下满足N*Ts不超过0.5。

extern real32_T velI_xy;               // Variable: velI_xy
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  水平速度 I 增益。消除速度稳态误差

extern real32_T velI_z;                // Variable: velI_z
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  垂直速度 I 增益

extern real32_T velP_xy;               // Variable: velP_xy
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  水平速度 P 增益。速度误差→加速度指令

extern real32_T velP_z;                // Variable: velP_z
                                          //  Referenced by: '<Root>/UAV_FlightControl'
                                          //  垂直速度 P 增益


// Model reference registration function
extern void FlightCore_initialize(const char_T **rt_errorStatus, RTWSolverInfo
  *rt_solverInfo, const rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T
  mdlref_TID1, int_T mdlref_TID2, int_T mdlref_TID3, RT_MODEL_FlightCore_T *
  const FlightCore_M, DW_FlightCore_f_T *localDW, ZCE_FlightCore_T *localZCE);
extern void FlightCore_Init(DW_FlightCore_f_T *localDW);
extern void FlightCore_Disable(DW_FlightCore_f_T *localDW);
extern void FlightCore(RT_MODEL_FlightCore_T * const FlightCore_M, const
  real32_T rtu_IMU_BUS_Accel[3], const real32_T rtu_IMU_BUS_Gyro[3], const
  boolean_T *rtu_IMU_BUS_Valid, const real32_T *rtu_GPS_BUS_Lat, const real32_T *
  rtu_GPS_BUS_Lon, const real32_T *rtu_GPS_BUS_Alt, const real32_T
  rtu_GPS_BUS_Velocity[3], const boolean_T *rtu_GPS_BUS_Valid, const boolean_T
  *rtu_GPS_BUS_IsNew, real32_T rty_EscCmdBus_MotorCmd[4], boolean_T
  *rty_EscCmdBus_Armed, boolean_T *rty_EscCmdBus_Valid, real32_T
  rty_StateEstBus_Position_NED[3], real32_T rty_StateEstBus_Velocity_NED[3],
  real32_T rty_StateEstBus_Attitude_quat[4], real32_T
  rty_StateEstBus_AngularRate_Bod[3], real32_T rty_StateEstBus_Accel_Body[3],
  real32_T rty_StateEstBus_GyroBias[3], real32_T rty_StateEstBus_AccelBias[3],
  real32_T rty_StateEstBus_Wind_NED[3], uint8_T *rty_StateEstBus_Status,
  B_FlightCore_c_T *localB, DW_FlightCore_f_T *localDW);

//-
//  The generated code includes comments that allow you to trace directly
//  back to the appropriate location in the model.  The basic format
//  is <system>/block_name, where system is the system number (uniquely
//  assigned by Simulink) and block_name is the name of the block.
//
//  Use the MATLAB hilite_system command to trace the generated code back
//  to the model.  For example,
//
//  hilite_system('<S3>')    - opens system 3
//  hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
//
//  Here is the system hierarchy for this model
//
//  '<Root>' : 'FlightCore'
//  '<S1>'   : 'FlightCore/Command'
//  '<S2>'   : 'FlightCore/Commander'
//  '<S3>'   : 'FlightCore/Commander/Chart'
//  '<S4>'   : 'FlightCore/Commander/Compare To Constant'

#endif                                 // FlightCore_h_

//
// File trailer for generated code.
//
// [EOF]
//
