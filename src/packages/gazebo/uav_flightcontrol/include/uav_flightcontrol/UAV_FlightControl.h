//
// File: UAV_FlightControl.h
//
// Code generated for Simulink model 'UAV_FlightControl'.
//
// Model version                  : 4.42
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Wed Jul 29 15:49:42 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef UAV_FlightControl_h_
#define UAV_FlightControl_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "UAV_FlightControl_types.h"
#include "model_reference_types.h"

// Block signals for model 'UAV_FlightControl'
struct B_UAV_FlightControl_c_T {
  real32_T R_error_Body[9];
  real32_T skew_error_Body[9];
  real32_T b2_des_NED[9];
  real32_T motor_cmd[4];               // '<S1>/Mixer'
  real32_T rtb_RT_ThrustCmd_50To1kHz_m[4];
  real32_T fv[4];
  real32_T b2_des_NED_c[3];
  real32_T RT_RateSP_250To1kHz[3];     // '<S2>/RT_RateSP_250To1kHz'
  real32_T q_norm;
  real32_T b2_norm;
  real32_T scale;
  real32_T absxk;
  real32_T t;
  real32_T IntegralGain;               // '<S100>/Integral Gain'
  real32_T IntegralGain_f;             // '<S156>/Integral Gain'
  real32_T DeadZone;                   // '<S93>/DeadZone'
  real32_T IntegralGain_h;             // '<S212>/Integral Gain'
  real32_T DeadZone_i;                 // '<S149>/DeadZone'
  real32_T DeadZone_o;                 // '<S205>/DeadZone'
  real32_T RT_ThrustCmd_50To1kHz;      // '<S1>/RT_ThrustCmd_50To1kHz'
  real32_T IntegralGain_l;             // '<S545>/Integral Gain'
  real32_T thrust_sp;                  // '<S5>/CollectiveThrust'
  real32_T Sat_acc_vert;               // '<S5>/Sat_acc_vert'
  real32_T IntegralGain_m;             // '<S430>/Integral Gain'
  real32_T DeadZone_gb;                // '<S423>/DeadZone'
  real32_T TmpSignalConversionAtFilterDiff;
  real32_T q_idx_2;
  real32_T q_idx_1;
  int32_T i;
};

// Block states (default storage) for model 'UAV_FlightControl'
struct DW_UAV_FlightControl_f_T {
  real32_T Integrator_DSTATE;          // '<S103>/Integrator'
  real32_T FilterDifferentiatorTF_states;// '<S96>/Filter Differentiator TF'
  real32_T Integrator_DSTATE_g;        // '<S159>/Integrator'
  real32_T FilterDifferentiatorTF_states_h;// '<S152>/Filter Differentiator TF'
  real32_T Integrator_DSTATE_a;        // '<S215>/Integrator'
  real32_T FilterDifferentiatorTF_states_l;// '<S208>/Filter Differentiator TF'
  real32_T Integrator_DSTATE_k[2];     // '<S377>/Integrator'
  real32_T FilterDifferentiatorTF_state_hl[2];// '<S370>/Filter Differentiator TF' 
  real32_T Integrator_DSTATE_m[2];     // '<S492>/Integrator'
  real32_T FilterDifferentiatorTF_states_m[2];// '<S485>/Filter Differentiator TF' 
  real32_T Integrator_DSTATE_o;        // '<S433>/Integrator'
  real32_T FilterDifferentiatorTF_states_a;// '<S426>/Filter Differentiator TF'
  real32_T FilterDifferentiatorTF_states_e;// '<S541>/Filter Differentiator TF'
  real32_T Integrator_DSTATE_p;        // '<S548>/Integrator'
  volatile real32_T RT_RateSP_250To1kHz_Buffer[6];// '<S2>/RT_RateSP_250To1kHz'
  real32_T FilterDifferentiatorTF_tmp; // '<S96>/Filter Differentiator TF'
  real32_T FilterDifferentiatorTF_tmp_b;// '<S152>/Filter Differentiator TF'
  real32_T FilterDifferentiatorTF_tmp_g;// '<S208>/Filter Differentiator TF'
  volatile real32_T RT_ThrustCmd_50To1kHz_Buffer0;// '<S1>/RT_ThrustCmd_50To1kHz' 
  volatile real32_T RT_AccCmd_50To250Hz_Buffer[6];// '<S1>/RT_AccCmd_50To250Hz'
  real32_T RT_AttitudeEst_1kTo250Hz_Buffer[4];// '<S1>/RT_AttitudeEst_1kTo250Hz' 
  real32_T RT_YawCmd_1kTo250Hz_Buffer; // '<S1>/RT_YawCmd_1kTo250Hz'
  real32_T RT_PositionCmd_1kTo50Hz_Buffer[3];// '<S1>/RT_PositionCmd_1kTo50Hz'
  real32_T RT_PositionEst_1kTo50Hz_Buffer[3];// '<S1>/RT_PositionEst_1kTo50Hz'
  real32_T FilterDifferentiatorTF_tmp_j[2];// '<S370>/Filter Differentiator TF'
  real32_T RT_VelocityEst_1kTo50Hz_Buffer[3];// '<S1>/RT_VelocityEst_1kTo50Hz'
  real32_T FilterDifferentiatorTF_tmp_c[2];// '<S485>/Filter Differentiator TF'
  real32_T FilterDifferentiatorTF_tmp_p;// '<S426>/Filter Differentiator TF'
  real32_T FilterDifferentiatorTF_tmp_m;// '<S541>/Filter Differentiator TF'
  volatile int8_T RT_RateSP_250To1kHz_ActiveBufId;// '<S2>/RT_RateSP_250To1kHz'
  volatile int8_T RT_AccCmd_50To250Hz_ActiveBufId;// '<S1>/RT_AccCmd_50To250Hz'
  boolean_T ControlLaw_MODE;           // '<Root>/ControlLaw'
};

// Real-time Model Data Structure
struct tag_RTM_UAV_FlightControl_T {
  const char_T **errorStatus;
  const rtTimingBridge *timingBridge;

  //
  //  Timing:
  //  The following substructure contains information regarding
  //  the timing information for the model.

  struct {
    int_T mdlref_GlobalTID[3];
  } Timing;

  time_T getClockTickH0() const;
  time_T getClockTick0() const;
  time_T getClockTickH1() const;
  time_T getClockTick1() const;
  time_T getClockTickH2() const;
  time_T getClockTick2() const;
  const char_T* getErrorStatus() const;
  void setErrorStatus(const char_T* const aErrorStatus) const;
  const char_T** getErrorStatusPointer() const;
  void setErrorStatusPointer(const char_T** aErrorStatusPointer);
  boolean_T isSampleHit(int32_T sti) const;
  time_T getT() const;
};

struct MdlrefDW_UAV_FlightControl_T {
  B_UAV_FlightControl_c_T rtb;
  DW_UAV_FlightControl_f_T rtdw;
  RT_MODEL_UAV_FlightControl_T rtm;
};

//
//  Exported Global Parameters
//
//  Note: Exported global parameters are tunable parameters with an exported
//  global storage class designation.  Code generation will declare the memory for
//  these parameters and exports their symbols.
//

extern real32_T attP;                  // Variable: attP
                                          //  Referenced by:
                                          //    '<S52>/Proportional Gain'
                                          //    '<S272>/Proportional Gain'
                                          //    '<S324>/Proportional Gain'
                                          //  姿态 P 增益。姿态误差→角速率指令。只用P，I由角速率环承担，防止两级积分器串联振荡

extern real32_T mass;                  // Variable: mass
                                          //  Referenced by: '<S5>/Constant1'
                                          //  AirSim Generic F450 四旋翼总质量（含电池、载荷）。来源: MultiRotorParams::setupFrameGenericQuad

extern real32_T maxAcc_xy;             // Variable: maxAcc_xy
                                          //  Referenced by:
                                          //    '<S5>/Sat_acc_hori'
                                          //    '<S499>/Saturation'
                                          //    '<S482>/DeadZone'
                                          //  水平加速度指令上限。限制最大姿态倾斜角

extern real32_T maxAcc_z;              // Variable: maxAcc_z
                                          //  Referenced by:
                                          //    '<S5>/Sat_acc_vert'
                                          //    '<S555>/Saturation'
                                          //    '<S538>/DeadZone'
                                          //  垂直加速度指令上限

extern real32_T maxRate;               // Variable: maxRate
                                          //  Referenced by:
                                          //    '<S2>/Sat_RateP'
                                          //    '<S2>/Sat_RateQ'
                                          //    '<S2>/Sat_RateR'
                                          //  最大角速率指令 (= π rad/s ≈ 180°/s)

extern real32_T maxTorque;             // Variable: maxTorque
                                          //  Referenced by:
                                          //    '<S2>/Sat_TorqueX'
                                          //    '<S2>/Sat_TorqueY'
                                          //    '<S2>/Sat_TorqueZ'
                                          //    '<S110>/Saturation'
                                          //    '<S166>/Saturation'
                                          //    '<S222>/Saturation'
                                          //    '<S93>/DeadZone'
                                          //    '<S149>/DeadZone'
                                          //    '<S205>/DeadZone'
                                          //  单轴力矩指令上限。在电机饱和前拦截过大指令，确保控制在线性区

extern real32_T maxVel_xy;             // Variable: maxVel_xy
                                          //  Referenced by:
                                          //    '<S4>/Sat_vel_hori'
                                          //    '<S384>/Saturation'
                                          //    '<S367>/DeadZone'
                                          //  水平速度指令上限

extern real32_T maxVel_z;              // Variable: maxVel_z
                                          //  Referenced by:
                                          //    '<S4>/Sat_vel_vert'
                                          //    '<S440>/Saturation'
                                          //    '<S423>/DeadZone'
                                          //  垂直速度指令上限

extern real32_T mixMatrix[16];         // Variable: mixMatrix
                                          //  Referenced by: '<S1>/MixMatrix'
                                          //  X型四旋翼混控矩阵 (4×4)。[F, τx, τy, τz]&#x1D40; → [m1, m2, m3, m4]&#x1D40;。列分别对应: 总推力、滚转力矩、俯仰力矩、偏航力矩

extern real32_T motorArmLength_m;      // Variable: motorArmLength_m
                                          //  Referenced by: '<S1>/MotorArmLength_m'
                                          //  AirSim Generic F450 电机到重心的水平距离（臂长）

extern real32_T motorMax;              // Variable: motorMax
                                          //  Referenced by: '<S1>/MotorMax'
                                          //  电机指令最大值（归一化 0~1）。硬件保护上限

extern real32_T motorMaxReactionTorque_Nm;// Variable: motorMaxReactionTorque_Nm
                                             //  Referenced by: '<S1>/MotorMaxReactionTorque_Nm'
                                             //  AirSim Generic Quad 单旋翼最大反扭矩。normalized command 1.0 线性对应此反扭矩

extern real32_T motorMaxThrust_N;      // Variable: motorMaxThrust_N
                                          //  Referenced by: '<S1>/MotorMaxThrust_N'
                                          //  AirSim Generic Quad 单旋翼最大推力。normalized command 1.0 线性对应此推力

extern real32_T motorMin;              // Variable: motorMin
                                          //  Referenced by: '<S1>/MotorMin'
                                          //  电机指令最小值（归一化 0~1）。>0 防止电机停转失去姿态调控能力

extern real32_T posD_xy;               // Variable: posD_xy
                                          //  Referenced by: '<S368>/Derivative Gain'
                                          //  水平位置 D 增益。当前=0（微分由角速率环 D 提供）

extern real32_T posD_z;                // Variable: posD_z
                                          //  Referenced by: '<S424>/Derivative Gain'
                                          //  高度位置 D 增益。当前=0

extern real32_T posFilterN;            // Variable: posFilterN
                                          //  Referenced by:
                                          //    '<S379>/N Copy'
                                          //    '<S380>/Filter Coefficient'
                                          //    '<S435>/N Copy'
                                          //    '<S436>/Filter Coefficient'
                                          //  位置PID导数滤波系数；在50 Hz周期下满足N*Ts不超过0.5。

extern real32_T posI_xy;               // Variable: posI_xy
                                          //  Referenced by: '<S374>/Integral Gain'
                                          //  水平位置 I 增益。消除悬停稳态位置误差和常值风扰

extern real32_T posI_z;                // Variable: posI_z
                                          //  Referenced by: '<S430>/Integral Gain'
                                          //  高度位置 I 增益

extern real32_T posP_xy;               // Variable: posP_xy
                                          //  Referenced by: '<S382>/Proportional Gain'
                                          //  水平位置 P 增益。位置误差→速度指令。增大加快位置响应，过大引发超调

extern real32_T posP_z;                // Variable: posP_z
                                          //  Referenced by: '<S438>/Proportional Gain'
                                          //  高度位置 P 增益。垂直增益 > 水平增益以积极对抗重力偏差

extern real32_T rateD;                 // Variable: rateD
                                          //  Referenced by:
                                          //    '<S94>/Derivative Gain'
                                          //    '<S150>/Derivative Gain'
                                          //    '<S206>/Derivative Gain'
                                          //  角速率 D 增益。提供角速率阻尼，抑制姿态修正时的振荡

extern real32_T rateFilterN;           // Variable: rateFilterN
                                          //  Referenced by:
                                          //    '<S105>/N Copy'
                                          //    '<S106>/Filter Coefficient'
                                          //    '<S161>/N Copy'
                                          //    '<S162>/Filter Coefficient'
                                          //    '<S217>/N Copy'
                                          //    '<S218>/Filter Coefficient'
                                          //  角速率PID导数滤波器系数。

extern real32_T rateI;                 // Variable: rateI
                                          //  Referenced by:
                                          //    '<S100>/Integral Gain'
                                          //    '<S156>/Integral Gain'
                                          //    '<S212>/Integral Gain'
                                          //  角速率 I 增益。消除角速率稳态误差、补偿常值扰动力矩

extern real32_T rateP;                 // Variable: rateP
                                          //  Referenced by:
                                          //    '<S108>/Proportional Gain'
                                          //    '<S164>/Proportional Gain'
                                          //    '<S220>/Proportional Gain'
                                          //  角速率 P 增益。对抗角速率扰动（突风、重心偏移）

extern real32_T velD_xy;               // Variable: velD_xy
                                          //  Referenced by: '<S483>/Derivative Gain'
                                          //  水平速度 D 增益。当前=0

extern real32_T velD_z;                // Variable: velD_z
                                          //  Referenced by: '<S539>/Derivative Gain'
                                          //  垂直速度 D 增益。当前=0

extern real32_T velFilterN;            // Variable: velFilterN
                                          //  Referenced by:
                                          //    '<S494>/N Copy'
                                          //    '<S495>/Filter Coefficient'
                                          //    '<S550>/N Copy'
                                          //    '<S551>/Filter Coefficient'
                                          //  速度PID导数滤波系数；在50 Hz周期下满足N*Ts不超过0.5。

extern real32_T velI_xy;               // Variable: velI_xy
                                          //  Referenced by: '<S489>/Integral Gain'
                                          //  水平速度 I 增益。消除速度稳态误差

extern real32_T velI_z;                // Variable: velI_z
                                          //  Referenced by: '<S545>/Integral Gain'
                                          //  垂直速度 I 增益

extern real32_T velP_xy;               // Variable: velP_xy
                                          //  Referenced by: '<S497>/Proportional Gain'
                                          //  水平速度 P 增益。速度误差→加速度指令

extern real32_T velP_z;                // Variable: velP_z
                                          //  Referenced by: '<S553>/Proportional Gain'
                                          //  垂直速度 P 增益


// Model reference registration function
extern void UAV_FlightControl_initialize(const char_T **rt_errorStatus, const
  rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T mdlref_TID1, int_T
  mdlref_TID2, RT_MODEL_UAV_FlightControl_T *const UAV_FlightControl_M);
extern void UAV_FlightControl_Init(DW_UAV_FlightControl_f_T *localDW);
extern void UAV_FlightControl_Disable(B_UAV_FlightControl_c_T *localB,
  DW_UAV_FlightControl_f_T *localDW);
extern void UAV_FlightControl(RT_MODEL_UAV_FlightControl_T * const
  UAV_FlightControl_M, const StateEstBus *rtu_StateEstBus, const real32_T
  rtu_FlightCmdBus_Position_NED_S[3], const real32_T *rtu_FlightCmdBus_Yaw_SP,
  const boolean_T *rtu_ControlActive, const boolean_T *rtu_Armed, EscCmdBus
  *rty_EscCmdBus, B_UAV_FlightControl_c_T *localB, DW_UAV_FlightControl_f_T
  *localDW);

//-
//  These blocks were eliminated from the model due to optimizations:
//
//  Block '<S96>/Passthrough for tuning' : Eliminate redundant data type conversion
//  Block '<S152>/Passthrough for tuning' : Eliminate redundant data type conversion
//  Block '<S208>/Passthrough for tuning' : Eliminate redundant data type conversion
//  Block '<S370>/Passthrough for tuning' : Eliminate redundant data type conversion
//  Block '<S426>/Passthrough for tuning' : Eliminate redundant data type conversion
//  Block '<S485>/Passthrough for tuning' : Eliminate redundant data type conversion
//  Block '<S541>/Passthrough for tuning' : Eliminate redundant data type conversion


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
//  '<Root>' : 'UAV_FlightControl'
//  '<S1>'   : 'UAV_FlightControl/ControlLaw'
//  '<S2>'   : 'UAV_FlightControl/ControlLaw/AttitudeRate'
//  '<S3>'   : 'UAV_FlightControl/ControlLaw/Mixer'
//  '<S4>'   : 'UAV_FlightControl/ControlLaw/PositionLoop'
//  '<S5>'   : 'UAV_FlightControl/ControlLaw/VelocityLoop'
//  '<S6>'   : 'UAV_FlightControl/ControlLaw/AttitudeRate/AccelToAttitude'
//  '<S7>'   : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch'
//  '<S8>'   : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP'
//  '<S9>'   : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ'
//  '<S10>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR'
//  '<S11>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll'
//  '<S12>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw'
//  '<S13>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Anti-windup'
//  '<S14>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/D Gain'
//  '<S15>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/External Derivative'
//  '<S16>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Filter'
//  '<S17>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Filter ICs'
//  '<S18>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/I Gain'
//  '<S19>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Ideal P Gain'
//  '<S20>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Ideal P Gain Fdbk'
//  '<S21>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Integrator'
//  '<S22>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Integrator ICs'
//  '<S23>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/N Copy'
//  '<S24>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/N Gain'
//  '<S25>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/P Copy'
//  '<S26>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Parallel P Gain'
//  '<S27>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Reset Signal'
//  '<S28>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Saturation'
//  '<S29>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Saturation Fdbk'
//  '<S30>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Sum'
//  '<S31>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Sum Fdbk'
//  '<S32>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Tracking Mode'
//  '<S33>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Tracking Mode Sum'
//  '<S34>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Tsamp - Integral'
//  '<S35>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Tsamp - Ngain'
//  '<S36>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/postSat Signal'
//  '<S37>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/preInt Signal'
//  '<S38>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/preSat Signal'
//  '<S39>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Anti-windup/Disabled'
//  '<S40>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/D Gain/Disabled'
//  '<S41>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/External Derivative/Disabled'
//  '<S42>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Filter/Disabled'
//  '<S43>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Filter ICs/Disabled'
//  '<S44>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/I Gain/Disabled'
//  '<S45>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Ideal P Gain/Passthrough'
//  '<S46>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Ideal P Gain Fdbk/Disabled'
//  '<S47>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Integrator/Disabled'
//  '<S48>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Integrator ICs/Disabled'
//  '<S49>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/N Copy/Disabled wSignal Specification'
//  '<S50>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/N Gain/Disabled'
//  '<S51>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/P Copy/Disabled'
//  '<S52>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Parallel P Gain/Internal Parameters'
//  '<S53>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Reset Signal/Disabled'
//  '<S54>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Saturation/Passthrough'
//  '<S55>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Saturation Fdbk/Disabled'
//  '<S56>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Sum/Passthrough_P'
//  '<S57>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Sum Fdbk/Disabled'
//  '<S58>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Tracking Mode/Disabled'
//  '<S59>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Tracking Mode Sum/Passthrough'
//  '<S60>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Tsamp - Integral/TsSignalSpecification'
//  '<S61>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/Tsamp - Ngain/Passthrough'
//  '<S62>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/postSat Signal/Forward_Path'
//  '<S63>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/preInt Signal/Internal PreInt'
//  '<S64>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Pitch/preSat Signal/Forward_Path'
//  '<S65>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Anti-windup'
//  '<S66>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/D Gain'
//  '<S67>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/External Derivative'
//  '<S68>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Filter'
//  '<S69>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Filter ICs'
//  '<S70>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/I Gain'
//  '<S71>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Ideal P Gain'
//  '<S72>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Ideal P Gain Fdbk'
//  '<S73>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Integrator'
//  '<S74>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Integrator ICs'
//  '<S75>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/N Copy'
//  '<S76>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/N Gain'
//  '<S77>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/P Copy'
//  '<S78>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Parallel P Gain'
//  '<S79>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Reset Signal'
//  '<S80>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Saturation'
//  '<S81>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Saturation Fdbk'
//  '<S82>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Sum'
//  '<S83>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Sum Fdbk'
//  '<S84>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Tracking Mode'
//  '<S85>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Tracking Mode Sum'
//  '<S86>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Tsamp - Integral'
//  '<S87>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Tsamp - Ngain'
//  '<S88>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/postSat Signal'
//  '<S89>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/preInt Signal'
//  '<S90>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/preSat Signal'
//  '<S91>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Anti-windup/Disc. Clamping Parallel'
//  '<S92>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Anti-windup/Disc. Clamping Parallel/Dead Zone'
//  '<S93>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
//  '<S94>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/D Gain/Internal Parameters'
//  '<S95>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/External Derivative/Error'
//  '<S96>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Filter/Disc. Backward Euler Filter'
//  '<S97>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Filter/Disc. Backward Euler Filter/Tsamp'
//  '<S98>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Filter/Disc. Backward Euler Filter/Tsamp/Internal Ts'
//  '<S99>'  : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Filter ICs/Internal IC - Filter'
//  '<S100>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/I Gain/Internal Parameters'
//  '<S101>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Ideal P Gain/Passthrough'
//  '<S102>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Ideal P Gain Fdbk/Disabled'
//  '<S103>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Integrator/Discrete'
//  '<S104>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Integrator ICs/Internal IC'
//  '<S105>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/N Copy/Internal Parameters'
//  '<S106>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/N Gain/Internal Parameters'
//  '<S107>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/P Copy/Disabled'
//  '<S108>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Parallel P Gain/Internal Parameters'
//  '<S109>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Reset Signal/Disabled'
//  '<S110>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Saturation/Enabled'
//  '<S111>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Saturation Fdbk/Disabled'
//  '<S112>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Sum/Sum_PID'
//  '<S113>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Sum Fdbk/Disabled'
//  '<S114>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Tracking Mode/Disabled'
//  '<S115>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Tracking Mode Sum/Passthrough'
//  '<S116>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Tsamp - Integral/TsSignalSpecification'
//  '<S117>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/Tsamp - Ngain/Passthrough'
//  '<S118>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/postSat Signal/Forward_Path'
//  '<S119>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/preInt Signal/Internal PreInt'
//  '<S120>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateP/preSat Signal/Forward_Path'
//  '<S121>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Anti-windup'
//  '<S122>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/D Gain'
//  '<S123>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/External Derivative'
//  '<S124>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Filter'
//  '<S125>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Filter ICs'
//  '<S126>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/I Gain'
//  '<S127>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Ideal P Gain'
//  '<S128>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Ideal P Gain Fdbk'
//  '<S129>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Integrator'
//  '<S130>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Integrator ICs'
//  '<S131>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/N Copy'
//  '<S132>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/N Gain'
//  '<S133>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/P Copy'
//  '<S134>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Parallel P Gain'
//  '<S135>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Reset Signal'
//  '<S136>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Saturation'
//  '<S137>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Saturation Fdbk'
//  '<S138>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Sum'
//  '<S139>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Sum Fdbk'
//  '<S140>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Tracking Mode'
//  '<S141>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Tracking Mode Sum'
//  '<S142>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Tsamp - Integral'
//  '<S143>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Tsamp - Ngain'
//  '<S144>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/postSat Signal'
//  '<S145>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/preInt Signal'
//  '<S146>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/preSat Signal'
//  '<S147>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Anti-windup/Disc. Clamping Parallel'
//  '<S148>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Anti-windup/Disc. Clamping Parallel/Dead Zone'
//  '<S149>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
//  '<S150>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/D Gain/Internal Parameters'
//  '<S151>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/External Derivative/Error'
//  '<S152>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Filter/Disc. Backward Euler Filter'
//  '<S153>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Filter/Disc. Backward Euler Filter/Tsamp'
//  '<S154>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Filter/Disc. Backward Euler Filter/Tsamp/Internal Ts'
//  '<S155>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Filter ICs/Internal IC - Filter'
//  '<S156>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/I Gain/Internal Parameters'
//  '<S157>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Ideal P Gain/Passthrough'
//  '<S158>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Ideal P Gain Fdbk/Disabled'
//  '<S159>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Integrator/Discrete'
//  '<S160>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Integrator ICs/Internal IC'
//  '<S161>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/N Copy/Internal Parameters'
//  '<S162>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/N Gain/Internal Parameters'
//  '<S163>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/P Copy/Disabled'
//  '<S164>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Parallel P Gain/Internal Parameters'
//  '<S165>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Reset Signal/Disabled'
//  '<S166>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Saturation/Enabled'
//  '<S167>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Saturation Fdbk/Disabled'
//  '<S168>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Sum/Sum_PID'
//  '<S169>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Sum Fdbk/Disabled'
//  '<S170>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Tracking Mode/Disabled'
//  '<S171>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Tracking Mode Sum/Passthrough'
//  '<S172>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Tsamp - Integral/TsSignalSpecification'
//  '<S173>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/Tsamp - Ngain/Passthrough'
//  '<S174>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/postSat Signal/Forward_Path'
//  '<S175>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/preInt Signal/Internal PreInt'
//  '<S176>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateQ/preSat Signal/Forward_Path'
//  '<S177>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Anti-windup'
//  '<S178>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/D Gain'
//  '<S179>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/External Derivative'
//  '<S180>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Filter'
//  '<S181>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Filter ICs'
//  '<S182>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/I Gain'
//  '<S183>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Ideal P Gain'
//  '<S184>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Ideal P Gain Fdbk'
//  '<S185>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Integrator'
//  '<S186>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Integrator ICs'
//  '<S187>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/N Copy'
//  '<S188>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/N Gain'
//  '<S189>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/P Copy'
//  '<S190>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Parallel P Gain'
//  '<S191>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Reset Signal'
//  '<S192>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Saturation'
//  '<S193>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Saturation Fdbk'
//  '<S194>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Sum'
//  '<S195>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Sum Fdbk'
//  '<S196>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Tracking Mode'
//  '<S197>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Tracking Mode Sum'
//  '<S198>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Tsamp - Integral'
//  '<S199>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Tsamp - Ngain'
//  '<S200>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/postSat Signal'
//  '<S201>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/preInt Signal'
//  '<S202>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/preSat Signal'
//  '<S203>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Anti-windup/Disc. Clamping Parallel'
//  '<S204>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Anti-windup/Disc. Clamping Parallel/Dead Zone'
//  '<S205>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
//  '<S206>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/D Gain/Internal Parameters'
//  '<S207>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/External Derivative/Error'
//  '<S208>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Filter/Disc. Backward Euler Filter'
//  '<S209>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Filter/Disc. Backward Euler Filter/Tsamp'
//  '<S210>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Filter/Disc. Backward Euler Filter/Tsamp/Internal Ts'
//  '<S211>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Filter ICs/Internal IC - Filter'
//  '<S212>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/I Gain/Internal Parameters'
//  '<S213>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Ideal P Gain/Passthrough'
//  '<S214>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Ideal P Gain Fdbk/Disabled'
//  '<S215>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Integrator/Discrete'
//  '<S216>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Integrator ICs/Internal IC'
//  '<S217>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/N Copy/Internal Parameters'
//  '<S218>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/N Gain/Internal Parameters'
//  '<S219>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/P Copy/Disabled'
//  '<S220>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Parallel P Gain/Internal Parameters'
//  '<S221>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Reset Signal/Disabled'
//  '<S222>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Saturation/Enabled'
//  '<S223>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Saturation Fdbk/Disabled'
//  '<S224>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Sum/Sum_PID'
//  '<S225>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Sum Fdbk/Disabled'
//  '<S226>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Tracking Mode/Disabled'
//  '<S227>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Tracking Mode Sum/Passthrough'
//  '<S228>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Tsamp - Integral/TsSignalSpecification'
//  '<S229>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/Tsamp - Ngain/Passthrough'
//  '<S230>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/postSat Signal/Forward_Path'
//  '<S231>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/preInt Signal/Internal PreInt'
//  '<S232>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_RateR/preSat Signal/Forward_Path'
//  '<S233>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Anti-windup'
//  '<S234>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/D Gain'
//  '<S235>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/External Derivative'
//  '<S236>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Filter'
//  '<S237>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Filter ICs'
//  '<S238>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/I Gain'
//  '<S239>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Ideal P Gain'
//  '<S240>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Ideal P Gain Fdbk'
//  '<S241>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Integrator'
//  '<S242>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Integrator ICs'
//  '<S243>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/N Copy'
//  '<S244>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/N Gain'
//  '<S245>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/P Copy'
//  '<S246>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Parallel P Gain'
//  '<S247>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Reset Signal'
//  '<S248>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Saturation'
//  '<S249>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Saturation Fdbk'
//  '<S250>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Sum'
//  '<S251>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Sum Fdbk'
//  '<S252>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Tracking Mode'
//  '<S253>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Tracking Mode Sum'
//  '<S254>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Tsamp - Integral'
//  '<S255>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Tsamp - Ngain'
//  '<S256>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/postSat Signal'
//  '<S257>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/preInt Signal'
//  '<S258>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/preSat Signal'
//  '<S259>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Anti-windup/Disabled'
//  '<S260>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/D Gain/Disabled'
//  '<S261>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/External Derivative/Disabled'
//  '<S262>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Filter/Disabled'
//  '<S263>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Filter ICs/Disabled'
//  '<S264>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/I Gain/Disabled'
//  '<S265>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Ideal P Gain/Passthrough'
//  '<S266>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Ideal P Gain Fdbk/Disabled'
//  '<S267>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Integrator/Disabled'
//  '<S268>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Integrator ICs/Disabled'
//  '<S269>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/N Copy/Disabled wSignal Specification'
//  '<S270>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/N Gain/Disabled'
//  '<S271>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/P Copy/Disabled'
//  '<S272>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Parallel P Gain/Internal Parameters'
//  '<S273>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Reset Signal/Disabled'
//  '<S274>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Saturation/Passthrough'
//  '<S275>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Saturation Fdbk/Disabled'
//  '<S276>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Sum/Passthrough_P'
//  '<S277>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Sum Fdbk/Disabled'
//  '<S278>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Tracking Mode/Disabled'
//  '<S279>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Tracking Mode Sum/Passthrough'
//  '<S280>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Tsamp - Integral/TsSignalSpecification'
//  '<S281>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/Tsamp - Ngain/Passthrough'
//  '<S282>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/postSat Signal/Forward_Path'
//  '<S283>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/preInt Signal/Internal PreInt'
//  '<S284>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Roll/preSat Signal/Forward_Path'
//  '<S285>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Anti-windup'
//  '<S286>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/D Gain'
//  '<S287>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/External Derivative'
//  '<S288>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Filter'
//  '<S289>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Filter ICs'
//  '<S290>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/I Gain'
//  '<S291>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Ideal P Gain'
//  '<S292>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Ideal P Gain Fdbk'
//  '<S293>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Integrator'
//  '<S294>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Integrator ICs'
//  '<S295>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/N Copy'
//  '<S296>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/N Gain'
//  '<S297>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/P Copy'
//  '<S298>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Parallel P Gain'
//  '<S299>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Reset Signal'
//  '<S300>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Saturation'
//  '<S301>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Saturation Fdbk'
//  '<S302>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Sum'
//  '<S303>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Sum Fdbk'
//  '<S304>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Tracking Mode'
//  '<S305>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Tracking Mode Sum'
//  '<S306>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Tsamp - Integral'
//  '<S307>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Tsamp - Ngain'
//  '<S308>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/postSat Signal'
//  '<S309>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/preInt Signal'
//  '<S310>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/preSat Signal'
//  '<S311>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Anti-windup/Disabled'
//  '<S312>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/D Gain/Disabled'
//  '<S313>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/External Derivative/Disabled'
//  '<S314>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Filter/Disabled'
//  '<S315>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Filter ICs/Disabled'
//  '<S316>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/I Gain/Disabled'
//  '<S317>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Ideal P Gain/Passthrough'
//  '<S318>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Ideal P Gain Fdbk/Disabled'
//  '<S319>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Integrator/Disabled'
//  '<S320>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Integrator ICs/Disabled'
//  '<S321>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/N Copy/Disabled wSignal Specification'
//  '<S322>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/N Gain/Disabled'
//  '<S323>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/P Copy/Disabled'
//  '<S324>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Parallel P Gain/Internal Parameters'
//  '<S325>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Reset Signal/Disabled'
//  '<S326>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Saturation/Passthrough'
//  '<S327>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Saturation Fdbk/Disabled'
//  '<S328>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Sum/Passthrough_P'
//  '<S329>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Sum Fdbk/Disabled'
//  '<S330>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Tracking Mode/Disabled'
//  '<S331>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Tracking Mode Sum/Passthrough'
//  '<S332>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Tsamp - Integral/TsSignalSpecification'
//  '<S333>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/Tsamp - Ngain/Passthrough'
//  '<S334>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/postSat Signal/Forward_Path'
//  '<S335>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/preInt Signal/Internal PreInt'
//  '<S336>' : 'UAV_FlightControl/ControlLaw/AttitudeRate/PID_Yaw/preSat Signal/Forward_Path'
//  '<S337>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller'
//  '<S338>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1'
//  '<S339>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Anti-windup'
//  '<S340>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/D Gain'
//  '<S341>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/External Derivative'
//  '<S342>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Filter'
//  '<S343>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Filter ICs'
//  '<S344>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/I Gain'
//  '<S345>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Ideal P Gain'
//  '<S346>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Ideal P Gain Fdbk'
//  '<S347>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Integrator'
//  '<S348>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Integrator ICs'
//  '<S349>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/N Copy'
//  '<S350>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/N Gain'
//  '<S351>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/P Copy'
//  '<S352>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Parallel P Gain'
//  '<S353>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Reset Signal'
//  '<S354>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Saturation'
//  '<S355>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Saturation Fdbk'
//  '<S356>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Sum'
//  '<S357>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Sum Fdbk'
//  '<S358>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Tracking Mode'
//  '<S359>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Tracking Mode Sum'
//  '<S360>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Tsamp - Integral'
//  '<S361>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Tsamp - Ngain'
//  '<S362>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/postSat Signal'
//  '<S363>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/preInt Signal'
//  '<S364>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/preSat Signal'
//  '<S365>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Anti-windup/Disc. Clamping Parallel'
//  '<S366>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone'
//  '<S367>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
//  '<S368>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/D Gain/Internal Parameters'
//  '<S369>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/External Derivative/Error'
//  '<S370>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Filter/Disc. Backward Euler Filter'
//  '<S371>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Filter/Disc. Backward Euler Filter/Tsamp'
//  '<S372>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Filter/Disc. Backward Euler Filter/Tsamp/Internal Ts'
//  '<S373>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Filter ICs/Internal IC - Filter'
//  '<S374>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/I Gain/Internal Parameters'
//  '<S375>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Ideal P Gain/Passthrough'
//  '<S376>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Ideal P Gain Fdbk/Disabled'
//  '<S377>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Integrator/Discrete'
//  '<S378>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Integrator ICs/Internal IC'
//  '<S379>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/N Copy/Internal Parameters'
//  '<S380>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/N Gain/Internal Parameters'
//  '<S381>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/P Copy/Disabled'
//  '<S382>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Parallel P Gain/Internal Parameters'
//  '<S383>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Reset Signal/Disabled'
//  '<S384>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Saturation/Enabled'
//  '<S385>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Saturation Fdbk/Disabled'
//  '<S386>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Sum/Sum_PID'
//  '<S387>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Sum Fdbk/Disabled'
//  '<S388>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Tracking Mode/Disabled'
//  '<S389>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Tracking Mode Sum/Passthrough'
//  '<S390>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Tsamp - Integral/TsSignalSpecification'
//  '<S391>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/Tsamp - Ngain/Passthrough'
//  '<S392>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/postSat Signal/Forward_Path'
//  '<S393>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/preInt Signal/Internal PreInt'
//  '<S394>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller/preSat Signal/Forward_Path'
//  '<S395>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Anti-windup'
//  '<S396>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/D Gain'
//  '<S397>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/External Derivative'
//  '<S398>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Filter'
//  '<S399>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Filter ICs'
//  '<S400>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/I Gain'
//  '<S401>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Ideal P Gain'
//  '<S402>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Ideal P Gain Fdbk'
//  '<S403>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Integrator'
//  '<S404>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Integrator ICs'
//  '<S405>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/N Copy'
//  '<S406>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/N Gain'
//  '<S407>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/P Copy'
//  '<S408>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Parallel P Gain'
//  '<S409>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Reset Signal'
//  '<S410>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Saturation'
//  '<S411>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Saturation Fdbk'
//  '<S412>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Sum'
//  '<S413>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Sum Fdbk'
//  '<S414>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Tracking Mode'
//  '<S415>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Tracking Mode Sum'
//  '<S416>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Tsamp - Integral'
//  '<S417>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Tsamp - Ngain'
//  '<S418>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/postSat Signal'
//  '<S419>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/preInt Signal'
//  '<S420>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/preSat Signal'
//  '<S421>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Anti-windup/Disc. Clamping Parallel'
//  '<S422>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone'
//  '<S423>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
//  '<S424>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/D Gain/Internal Parameters'
//  '<S425>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/External Derivative/Error'
//  '<S426>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Filter/Disc. Backward Euler Filter'
//  '<S427>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Filter/Disc. Backward Euler Filter/Tsamp'
//  '<S428>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Filter/Disc. Backward Euler Filter/Tsamp/Internal Ts'
//  '<S429>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Filter ICs/Internal IC - Filter'
//  '<S430>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/I Gain/Internal Parameters'
//  '<S431>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Ideal P Gain/Passthrough'
//  '<S432>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Ideal P Gain Fdbk/Disabled'
//  '<S433>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Integrator/Discrete'
//  '<S434>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Integrator ICs/Internal IC'
//  '<S435>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/N Copy/Internal Parameters'
//  '<S436>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/N Gain/Internal Parameters'
//  '<S437>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/P Copy/Disabled'
//  '<S438>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Parallel P Gain/Internal Parameters'
//  '<S439>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Reset Signal/Disabled'
//  '<S440>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Saturation/Enabled'
//  '<S441>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Saturation Fdbk/Disabled'
//  '<S442>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Sum/Sum_PID'
//  '<S443>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Sum Fdbk/Disabled'
//  '<S444>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Tracking Mode/Disabled'
//  '<S445>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Tracking Mode Sum/Passthrough'
//  '<S446>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Tsamp - Integral/TsSignalSpecification'
//  '<S447>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/Tsamp - Ngain/Passthrough'
//  '<S448>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/postSat Signal/Forward_Path'
//  '<S449>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/preInt Signal/Internal PreInt'
//  '<S450>' : 'UAV_FlightControl/ControlLaw/PositionLoop/PID Controller1/preSat Signal/Forward_Path'
//  '<S451>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/CollectiveThrust'
//  '<S452>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller'
//  '<S453>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1'
//  '<S454>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Anti-windup'
//  '<S455>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/D Gain'
//  '<S456>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/External Derivative'
//  '<S457>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Filter'
//  '<S458>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Filter ICs'
//  '<S459>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/I Gain'
//  '<S460>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Ideal P Gain'
//  '<S461>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Ideal P Gain Fdbk'
//  '<S462>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Integrator'
//  '<S463>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Integrator ICs'
//  '<S464>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/N Copy'
//  '<S465>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/N Gain'
//  '<S466>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/P Copy'
//  '<S467>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Parallel P Gain'
//  '<S468>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Reset Signal'
//  '<S469>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Saturation'
//  '<S470>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Saturation Fdbk'
//  '<S471>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Sum'
//  '<S472>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Sum Fdbk'
//  '<S473>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Tracking Mode'
//  '<S474>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Tracking Mode Sum'
//  '<S475>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Tsamp - Integral'
//  '<S476>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Tsamp - Ngain'
//  '<S477>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/postSat Signal'
//  '<S478>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/preInt Signal'
//  '<S479>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/preSat Signal'
//  '<S480>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Anti-windup/Disc. Clamping Parallel'
//  '<S481>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone'
//  '<S482>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
//  '<S483>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/D Gain/Internal Parameters'
//  '<S484>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/External Derivative/Error'
//  '<S485>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Filter/Disc. Backward Euler Filter'
//  '<S486>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Filter/Disc. Backward Euler Filter/Tsamp'
//  '<S487>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Filter/Disc. Backward Euler Filter/Tsamp/Internal Ts'
//  '<S488>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Filter ICs/Internal IC - Filter'
//  '<S489>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/I Gain/Internal Parameters'
//  '<S490>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Ideal P Gain/Passthrough'
//  '<S491>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Ideal P Gain Fdbk/Disabled'
//  '<S492>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Integrator/Discrete'
//  '<S493>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Integrator ICs/Internal IC'
//  '<S494>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/N Copy/Internal Parameters'
//  '<S495>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/N Gain/Internal Parameters'
//  '<S496>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/P Copy/Disabled'
//  '<S497>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Parallel P Gain/Internal Parameters'
//  '<S498>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Reset Signal/Disabled'
//  '<S499>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Saturation/Enabled'
//  '<S500>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Saturation Fdbk/Disabled'
//  '<S501>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Sum/Sum_PID'
//  '<S502>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Sum Fdbk/Disabled'
//  '<S503>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Tracking Mode/Disabled'
//  '<S504>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Tracking Mode Sum/Passthrough'
//  '<S505>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Tsamp - Integral/TsSignalSpecification'
//  '<S506>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/Tsamp - Ngain/Passthrough'
//  '<S507>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/postSat Signal/Forward_Path'
//  '<S508>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/preInt Signal/Internal PreInt'
//  '<S509>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller/preSat Signal/Forward_Path'
//  '<S510>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Anti-windup'
//  '<S511>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/D Gain'
//  '<S512>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/External Derivative'
//  '<S513>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Filter'
//  '<S514>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Filter ICs'
//  '<S515>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/I Gain'
//  '<S516>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Ideal P Gain'
//  '<S517>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Ideal P Gain Fdbk'
//  '<S518>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Integrator'
//  '<S519>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Integrator ICs'
//  '<S520>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/N Copy'
//  '<S521>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/N Gain'
//  '<S522>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/P Copy'
//  '<S523>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Parallel P Gain'
//  '<S524>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Reset Signal'
//  '<S525>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Saturation'
//  '<S526>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Saturation Fdbk'
//  '<S527>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Sum'
//  '<S528>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Sum Fdbk'
//  '<S529>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Tracking Mode'
//  '<S530>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Tracking Mode Sum'
//  '<S531>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Tsamp - Integral'
//  '<S532>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Tsamp - Ngain'
//  '<S533>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/postSat Signal'
//  '<S534>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/preInt Signal'
//  '<S535>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/preSat Signal'
//  '<S536>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Anti-windup/Disc. Clamping Parallel'
//  '<S537>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone'
//  '<S538>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Anti-windup/Disc. Clamping Parallel/Dead Zone/Enabled'
//  '<S539>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/D Gain/Internal Parameters'
//  '<S540>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/External Derivative/Error'
//  '<S541>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Filter/Disc. Backward Euler Filter'
//  '<S542>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Filter/Disc. Backward Euler Filter/Tsamp'
//  '<S543>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Filter/Disc. Backward Euler Filter/Tsamp/Internal Ts'
//  '<S544>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Filter ICs/Internal IC - Filter'
//  '<S545>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/I Gain/Internal Parameters'
//  '<S546>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Ideal P Gain/Passthrough'
//  '<S547>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Ideal P Gain Fdbk/Disabled'
//  '<S548>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Integrator/Discrete'
//  '<S549>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Integrator ICs/Internal IC'
//  '<S550>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/N Copy/Internal Parameters'
//  '<S551>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/N Gain/Internal Parameters'
//  '<S552>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/P Copy/Disabled'
//  '<S553>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Parallel P Gain/Internal Parameters'
//  '<S554>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Reset Signal/Disabled'
//  '<S555>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Saturation/Enabled'
//  '<S556>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Saturation Fdbk/Disabled'
//  '<S557>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Sum/Sum_PID'
//  '<S558>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Sum Fdbk/Disabled'
//  '<S559>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Tracking Mode/Disabled'
//  '<S560>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Tracking Mode Sum/Passthrough'
//  '<S561>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Tsamp - Integral/TsSignalSpecification'
//  '<S562>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/Tsamp - Ngain/Passthrough'
//  '<S563>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/postSat Signal/Forward_Path'
//  '<S564>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/preInt Signal/Internal PreInt'
//  '<S565>' : 'UAV_FlightControl/ControlLaw/VelocityLoop/PID Controller1/preSat Signal/Forward_Path'

#endif                                 // UAV_FlightControl_h_

//
// File trailer for generated code.
//
// [EOF]
//
