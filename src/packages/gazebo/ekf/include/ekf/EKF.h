//
// File: EKF.h
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
#ifndef EKF_h_
#define EKF_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "rt_nonfinite.h"
#include "EKF_types.h"

extern "C"
{

#include "rtGetNaN.h"

}

// Block signals for model 'EKF'
struct B_EKF_c_T {
  real_T a[225];
  real_T G[180];
  real32_T P_c[225];                   // '<S2>/EKF_Update'
  real32_T P_p[225];
  real32_T F[225];
  real32_T P_p_m[225];
  real32_T F_c[225];
  real32_T G_k[225];
  real32_T G_c[180];
  real32_T c_b[144];
  real32_T X[90];
  real32_T A_tmp[90];
  int8_T c_I[225];
  real32_T R[36];
  real32_T A[36];
  int8_T H[90];
  int8_T A_tmp_b[90];
  real32_T x_c[16];                    // '<S2>/EKF_Update'
  real32_T fv[16];
  real32_T delta_x[15];
  real32_T v[12];
  real32_T C_bn[9];
  real32_T w_sk[9];
  real32_T b_b[9];
  real32_T C_bn_p[9];
  real_T rtu_GPS_BUS_Lat[3];
  real_T dv[3];
  real32_T inertial[6];                // '<Root>/BiasCorrectedInertialOutput'
  real32_T fv1[6];
  real32_T accel_corr[6];
  real_T dv1[2];
  real32_T fv2[4];
  int8_T b_I[9];
  real_T cosphi;
  real_T sinphi;
  real_T coslambda;
  real_T sinlambda;
  real_T tmp;
  real_T ecefPosWithENUOrigin_idx_2;
  real_T sinphi_c;
  real_T N;
  real_T b;
  real_T c;
  real_T d;
  real_T b_x;
  real_T absx;
  real_T q;
  real_T b_x_f;
  real_T absx_g;
  int8_T ipiv[6];
  real32_T theta;
  real32_T e_b;
  real32_T smax;
  real32_T rtb_TmpSignalConversionAtSFun_g;
  real32_T rtb_TmpSignalConversionAtSFun_m;
  real32_T rtb_TmpSignalConversionAtSFun_n;
  real32_T delta_idx_0;
  real32_T delta_idx_1;
  real32_T delta_idx_2;
  real32_T accel_corr_idx_2;
  real32_T accel_corr_idx_1;
  real32_T accel_corr_idx_0;
  real32_T omega_corr_idx_1;
  real32_T omega_corr_idx_0;
  int32_T j;
  int32_T kBcol;
  int32_T jj;
  int32_T jA;
};

// Block states (default storage) for model 'EKF'
struct DW_EKF_f_T {
  real_T stored_origin[3];             // '<S3>/FirstFixOriginLatch'
  real32_T UnitDelay1_DSTATE[225];     // '<Root>/Unit Delay1'
  real32_T UnitDelay_DSTATE[16];       // '<Root>/Unit Delay'
  boolean_T initialized;               // '<S3>/FirstFixOriginLatch'
};

// Real-time Model Data Structure
struct tag_RTM_EKF_T {
  const char_T **errorStatus;
  const char_T* getErrorStatus() const;
  void setErrorStatus(const char_T* const aErrorStatus) const;
  const char_T** getErrorStatusPointer() const;
  void setErrorStatusPointer(const char_T** aErrorStatusPointer);
};

struct MdlrefDW_EKF_T {
  B_EKF_c_T rtb;
  DW_EKF_f_T rtdw;
  RT_MODEL_EKF_T rtm;
};

//
//  Exported Global Parameters
//
//  Note: Exported global parameters are tunable parameters with an exported
//  global storage class designation.  Code generation will declare the memory for
//  these parameters and exports their symbols.
//

extern real32_T R_pos;                 // Variable: R_pos
                                          //  Referenced by: '<S2>/R_pos'
                                          //  GPS 位置观测噪声标准差。增大→降低 GPS 位置在修正中的权重

extern real32_T R_vel;                 // Variable: R_vel
                                          //  Referenced by: '<S2>/R_vel'
                                          //  GPS 速度观测噪声标准差。增大→降低 GPS 速度在修正中的权重

extern real32_T SE_EKF_INIT_STATE[16]; // Variable: SE_EKF_INIT_STATE
                                          //  Referenced by: '<Root>/Unit Delay'
                                          //  EKF 16维默认名义状态 [Position_NED; Velocity_NED; Attitude_quat_wxyz; AccelBias; GyroBias]。位置以启动点为局部 NED 原点。

extern real32_T ekf_predict_dt;        // Variable: ekf_predict_dt
                                          //  Referenced by: '<Root>/ekf_predict_dt'
                                          //  EKF 预测步长数值输入。显式匹配 EKF MATLAB Function 的 single 数值契约

extern real32_T g_n[3];                // Variable: g_n
                                          //  Referenced by: '<Root>/g_n'
                                          //  NED 重力加速度矢量 [g_N, g_E, g_D]。物理常量，不作调参项

extern real32_T sigma_acc;             // Variable: sigma_acc
                                          //  Referenced by: '<Root>/sigma_acc_c'
                                          //  加速度计白噪声 PSD。增大→降低加速度计在融合中的权重

extern real32_T sigma_ba;              // Variable: sigma_ba
                                          //  Referenced by: '<Root>/sigma_ba_c'
                                          //  加速度计偏置随机游走 PSD。增大→允许偏置估计更快变化但噪声更大

extern real32_T sigma_bg;              // Variable: sigma_bg
                                          //  Referenced by: '<Root>/sigma_bg_c'
                                          //  陀螺仪偏置随机游走 PSD。增大→允许偏置估计更快变化但噪声更大

extern real32_T sigma_gyr;             // Variable: sigma_gyr
                                          //  Referenced by: '<Root>/sigma_gyr_c'
                                          //  陀螺仪白噪声 PSD。增大→降低陀螺仪在融合中的权重


// Model reference registration function
extern void EKF_initialize(const char_T **rt_errorStatus, RT_MODEL_EKF_T *const
  EKF_M);
extern void EKF_Init(DW_EKF_f_T *localDW);
extern void EKF(const real32_T rtu_IMU_BUS_Accel[3], const real32_T
                rtu_IMU_BUS_Gyro[3], const boolean_T *rtu_IMU_BUS_Valid, const
                real32_T *rtu_GPS_BUS_Lat, const real32_T *rtu_GPS_BUS_Lon,
                const real32_T *rtu_GPS_BUS_Alt, const real32_T
                rtu_GPS_BUS_Velocity[3], const boolean_T *rtu_GPS_BUS_Valid,
                const boolean_T *rtu_GPS_BUS_IsNew, real32_T
                rty_StateEstBus_Position_NED[3], real32_T
                rty_StateEstBus_Velocity_NED[3], real32_T
                rty_StateEstBus_Attitude_quat[4], real32_T
                rty_StateEstBus_AngularRate_Bod[3], real32_T
                rty_StateEstBus_Accel_Body[3], real32_T
                rty_StateEstBus_GyroBias[3], real32_T rty_StateEstBus_AccelBias
                [3], real32_T rty_StateEstBus_Wind_NED[3], uint8_T
                *rty_StateEstBus_Status, B_EKF_c_T *localB, DW_EKF_f_T *localDW);

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
//  '<Root>' : 'EKF'
//  '<S1>'   : 'EKF/BiasCorrectedInertialOutput'
//  '<S2>'   : 'EKF/EKFUpdate'
//  '<S3>'   : 'EKF/GPSPreprocess'
//  '<S4>'   : 'EKF/MATLAB Function'
//  '<S5>'   : 'EKF/StatePack'
//  '<S6>'   : 'EKF/EKFUpdate/EKF_Update'
//  '<S7>'   : 'EKF/GPSPreprocess/FirstFixOriginLatch'
//  '<S8>'   : 'EKF/GPSPreprocess/MATLAB Function'

#endif                                 // EKF_h_

//
// File trailer for generated code.
//
// [EOF]
//
