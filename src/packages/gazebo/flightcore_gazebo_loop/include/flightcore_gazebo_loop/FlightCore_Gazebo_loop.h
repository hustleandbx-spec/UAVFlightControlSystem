//
// File: FlightCore_Gazebo_loop.h
//
// Code generated for Simulink model 'FlightCore_Gazebo_loop'.
//
// Model version                  : 1.41
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Wed Jul 29 15:51:26 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef FlightCore_Gazebo_loop_h_
#define FlightCore_Gazebo_loop_h_
#include <cmath>
#include "rtwtypes.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "slros2_initialize.h"
#include "FlightCore_Gazebo_loop_types.h"
#include "FlightCore.h"
#include "model_reference_types.h"
#include "zero_crossing_types.h"

// Block signals (default storage)
struct B_FlightCore_Gazebo_loop_T {
  SL_Bus_flightcore_gazebo_msgs_ActuatorCommand ActuatorCommandAssign;// '<S1>/ActuatorCommandAssign' 
  SL_Bus_flightcore_msgs_Imu In1;      // '<S9>/In1'
  SL_Bus_flightcore_msgs_Gps In1_n;    // '<S8>/In1'
  SL_Bus_flightcore_msgs_Gps rtb_SourceBlock_o2_n_m;
  SL_Bus_flightcore_msgs_Imu rtb_SourceBlock_o2_c;
  char_T b_zeroDelimTopic[36];
  char_T b_zeroDelimTopic_k[23];
  char_T b_zeroDelimTopic_c[23];
  real32_T MotorCmd[4];                // '<Root>/FlightCore'
  real32_T Position_NED[3];            // '<Root>/FlightCore'
  real32_T Velocity_NED[3];            // '<Root>/FlightCore'
  real32_T Attitude_quat[4];           // '<Root>/FlightCore'
  real32_T AngularRate_Body[3];        // '<Root>/FlightCore'
  real32_T Accel_Body[3];              // '<Root>/FlightCore'
  real32_T GyroBias[3];                // '<Root>/FlightCore'
  real32_T AccelBias[3];               // '<Root>/FlightCore'
  real32_T Wind_NED[3];                // '<Root>/FlightCore'
  uint64_T qY;
  uint8_T Status;                      // '<Root>/FlightCore'
  boolean_T Armed;                     // '<Root>/FlightCore'
  boolean_T Valid;                     // '<Root>/FlightCore'
  boolean_T SourceBlock_o1;            // '<S5>/SourceBlock'
  boolean_T SourceBlock_o1_c;          // '<S3>/SourceBlock'
};

// Block states (default storage) for system '<Root>'
struct DW_FlightCore_Gazebo_loop_T {
  ros_slros2_internal_block_Pub_T obj; // '<S7>/SinkBlock'
  ros_slros2_internal_block_Sub_T obj_d;// '<S5>/SourceBlock'
  ros_slros2_internal_block_Sub_T obj_dn;// '<S3>/SourceBlock'
  uint64_T ExecutionCount_DSTATE;      // '<Root>/ExecutionCount'
  MdlrefDW_FlightCore_T FlightCore_InstanceData;// '<Root>/FlightCore'
};

// Real-time Model Data Structure
struct tag_RTM_FlightCore_Gazebo_loo_T {
  const char_T *errorStatus;
  RTWSolverInfo solverInfo;
  rtTimingBridge timingBridge;

  //
  //  Timing:
  //  The following substructure contains information regarding
  //  the timing information for the model.

  struct {
    uint32_T clockTick0;
    time_T stepSize0;
    uint32_T clockTick1;
    uint32_T clockTick2;
    uint32_T clockTick3;
    struct {
      uint32_T TID[4];
    } TaskCounters;

    SimTimeStep simTimeStep;
    time_T *t;
    time_T tArray[4];
  } Timing;

  time_T** getTPtrPtr();
  const char_T* getErrorStatus() const;
  void setErrorStatus(const char_T* const aErrorStatus);
  time_T* getTPtr() const;
  void setTPtr(time_T* aTPtr);
  const char_T** getErrorStatusPtr();
  const char_T** getErrorStatusPointer();
  boolean_T isMajorTimeStep() const;
  boolean_T isMinorTimeStep() const;
};

//
//  Exported Global Parameters
//
//  Note: Exported global parameters are tunable parameters with an exported
//  global storage class designation.  Code generation will declare the memory for
//  these parameters and exports their symbols.
//

extern real32_T R_pos;                 // Variable: R_pos
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  GPS 位置观测噪声标准差。增大→降低 GPS 位置在修正中的权重

extern real32_T R_vel;                 // Variable: R_vel
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  GPS 速度观测噪声标准差。增大→降低 GPS 速度在修正中的权重

extern real32_T SE_EKF_INIT_STATE[16]; // Variable: SE_EKF_INIT_STATE
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  EKF 16维默认名义状态 [Position_NED; Velocity_NED; Attitude_quat_wxyz; AccelBias; GyroBias]。位置以启动点为局部 NED 原点。

extern real32_T attP;                  // Variable: attP
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  姿态 P 增益。姿态误差→角速率指令。只用P，I由角速率环承担，防止两级积分器串联振荡

extern real32_T ekf_predict_dt;        // Variable: ekf_predict_dt
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  EKF 预测步长数值输入。显式匹配 EKF MATLAB Function 的 single 数值契约

extern real32_T g_n[3];                // Variable: g_n
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  NED 重力加速度矢量 [g_N, g_E, g_D]。物理常量，不作调参项

extern real32_T mass;                  // Variable: mass
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  AirSim Generic F450 四旋翼总质量（含电池、载荷）。来源: MultiRotorParams::setupFrameGenericQuad

extern real32_T maxAcc_xy;             // Variable: maxAcc_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平加速度指令上限。限制最大姿态倾斜角

extern real32_T maxAcc_z;              // Variable: maxAcc_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直加速度指令上限

extern real32_T maxRate;               // Variable: maxRate
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  最大角速率指令 (= π rad/s ≈ 180°/s)

extern real32_T maxTorque;             // Variable: maxTorque
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  单轴力矩指令上限。在电机饱和前拦截过大指令，确保控制在线性区

extern real32_T maxVel_xy;             // Variable: maxVel_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平速度指令上限

extern real32_T maxVel_z;              // Variable: maxVel_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直速度指令上限

extern real32_T mixMatrix[16];         // Variable: mixMatrix
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  X型四旋翼混控矩阵 (4×4)。[F, τx, τy, τz]&#x1D40; → [m1, m2, m3, m4]&#x1D40;。列分别对应: 总推力、滚转力矩、俯仰力矩、偏航力矩

extern real32_T motorArmLength_m;      // Variable: motorArmLength_m
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  AirSim Generic F450 电机到重心的水平距离（臂长）

extern real32_T motorMax;              // Variable: motorMax
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  电机指令最大值（归一化 0~1）。硬件保护上限

extern real32_T motorMaxReactionTorque_Nm;// Variable: motorMaxReactionTorque_Nm
                                             //  Referenced by: '<Root>/FlightCore'
                                             //  AirSim Generic Quad 单旋翼最大反扭矩。normalized command 1.0 线性对应此反扭矩

extern real32_T motorMaxThrust_N;      // Variable: motorMaxThrust_N
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  AirSim Generic Quad 单旋翼最大推力。normalized command 1.0 线性对应此推力

extern real32_T motorMin;              // Variable: motorMin
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  电机指令最小值（归一化 0~1）。>0 防止电机停转失去姿态调控能力

extern real32_T posD_xy;               // Variable: posD_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平位置 D 增益。当前=0（微分由角速率环 D 提供）

extern real32_T posD_z;                // Variable: posD_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  高度位置 D 增益。当前=0

extern real32_T posFilterN;            // Variable: posFilterN
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  位置PID导数滤波系数；在50 Hz周期下满足N*Ts不超过0.5。

extern real32_T posI_xy;               // Variable: posI_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平位置 I 增益。消除悬停稳态位置误差和常值风扰

extern real32_T posI_z;                // Variable: posI_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  高度位置 I 增益

extern real32_T posP_xy;               // Variable: posP_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平位置 P 增益。位置误差→速度指令。增大加快位置响应，过大引发超调

extern real32_T posP_z;                // Variable: posP_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  高度位置 P 增益。垂直增益 > 水平增益以积极对抗重力偏差

extern real32_T rateD;                 // Variable: rateD
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  角速率 D 增益。提供角速率阻尼，抑制姿态修正时的振荡

extern real32_T rateFilterN;           // Variable: rateFilterN
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  角速率PID导数滤波器系数。

extern real32_T rateI;                 // Variable: rateI
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  角速率 I 增益。消除角速率稳态误差、补偿常值扰动力矩

extern real32_T rateP;                 // Variable: rateP
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  角速率 P 增益。对抗角速率扰动（突风、重心偏移）

extern real32_T sigma_acc;             // Variable: sigma_acc
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  加速度计白噪声 PSD。增大→降低加速度计在融合中的权重

extern real32_T sigma_ba;              // Variable: sigma_ba
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  加速度计偏置随机游走 PSD。增大→允许偏置估计更快变化但噪声更大

extern real32_T sigma_bg;              // Variable: sigma_bg
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  陀螺仪偏置随机游走 PSD。增大→允许偏置估计更快变化但噪声更大

extern real32_T sigma_gyr;             // Variable: sigma_gyr
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  陀螺仪白噪声 PSD。增大→降低陀螺仪在融合中的权重

extern real32_T velD_xy;               // Variable: velD_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平速度 D 增益。当前=0

extern real32_T velD_z;                // Variable: velD_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直速度 D 增益。当前=0

extern real32_T velFilterN;            // Variable: velFilterN
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  速度PID导数滤波系数；在50 Hz周期下满足N*Ts不超过0.5。

extern real32_T velI_xy;               // Variable: velI_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平速度 I 增益。消除速度稳态误差

extern real32_T velI_z;                // Variable: velI_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直速度 I 增益

extern real32_T velP_xy;               // Variable: velP_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平速度 P 增益。速度误差→加速度指令

extern real32_T velP_z;                // Variable: velP_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直速度 P 增益


// Class declaration for model FlightCore_Gazebo_loop
class FlightCore_Gazebo_loop final
{
  // public data and function members
 public:
  // Copy Constructor
  FlightCore_Gazebo_loop(FlightCore_Gazebo_loop const&) = delete;

  // Assignment Operator
  FlightCore_Gazebo_loop& operator= (FlightCore_Gazebo_loop const&) & = delete;

  // Move Constructor
  FlightCore_Gazebo_loop(FlightCore_Gazebo_loop &&) = delete;

  // Move Assignment Operator
  FlightCore_Gazebo_loop& operator= (FlightCore_Gazebo_loop &&) = delete;

  // Real-Time Model get method
  RT_MODEL_FlightCore_Gazebo_lo_T * getRTM();

  // model initialize function
  void initialize();

  // model step function
  void step();

  // model terminate function
  void terminate();

  // Constructor
  FlightCore_Gazebo_loop();

  // Destructor
  ~FlightCore_Gazebo_loop();

  // private data and function members
 private:
  // Block signals
  B_FlightCore_Gazebo_loop_T FlightCore_Gazebo_loop_B;

  // Block states
  DW_FlightCore_Gazebo_loop_T FlightCore_Gazebo_loop_DW;

  // private member function(s) for subsystem '<Root>'
  void FlightCo_Subscriber_setupImpl_g(const ros_slros2_internal_block_Sub_T
    *obj);
  void FlightCore_Subscriber_setupImpl(const ros_slros2_internal_block_Sub_T
    *obj);
  void FlightCore__Publisher_setupImpl(const ros_slros2_internal_block_Pub_T
    *obj);

  // Real-Time Model
  RT_MODEL_FlightCore_Gazebo_lo_T FlightCore_Gazebo_loop_M;
};

extern volatile boolean_T stopRequested;
extern volatile boolean_T runModel;

//-
//  These blocks were eliminated from the model due to optimizations:
//
//  Block '<S1>/SourceStepUint64' : Eliminate redundant data type conversion
//  Block '<S1>/TargetIterationUint64' : Eliminate redundant data type conversion
//  Block '<S1>/TargetStepUint64' : Eliminate redundant data type conversion
//  Block '<S2>/Gazebo_GPS_Alt' : Eliminate redundant data type conversion
//  Block '<S2>/Gazebo_GPS_IsNew' : Eliminate redundant data type conversion
//  Block '<S2>/Gazebo_GPS_Lat' : Eliminate redundant data type conversion
//  Block '<S2>/Gazebo_GPS_Lon' : Eliminate redundant data type conversion
//  Block '<S2>/Gazebo_GPS_Timestamp' : Eliminate redundant data type conversion
//  Block '<S2>/Gazebo_GPS_Valid' : Eliminate redundant data type conversion
//  Block '<S2>/Gazebo_GPS_Velocity' : Eliminate redundant data type conversion
//  Block '<S4>/Gazebo_IMU_Accel' : Eliminate redundant data type conversion
//  Block '<S4>/Gazebo_IMU_Gyro' : Eliminate redundant data type conversion
//  Block '<S4>/Gazebo_IMU_IsNew' : Eliminate redundant data type conversion
//  Block '<S4>/Gazebo_IMU_Timestamp' : Eliminate redundant data type conversion
//  Block '<S4>/Gazebo_IMU_Valid' : Eliminate redundant data type conversion


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
//  '<Root>' : 'FlightCore_Gazebo_loop'
//  '<S1>'   : 'FlightCore_Gazebo_loop/GazeboEscCmd'
//  '<S2>'   : 'FlightCore_Gazebo_loop/GazeboGPS'
//  '<S3>'   : 'FlightCore_Gazebo_loop/GazeboGpsSubscribe'
//  '<S4>'   : 'FlightCore_Gazebo_loop/GazeboIMU'
//  '<S5>'   : 'FlightCore_Gazebo_loop/GazeboImuSubscribe'
//  '<S6>'   : 'FlightCore_Gazebo_loop/GazeboEscCmd/Gazebo_ESC_Blank_Message'
//  '<S7>'   : 'FlightCore_Gazebo_loop/GazeboEscCmd/Gazebo_ESC_Publish'
//  '<S8>'   : 'FlightCore_Gazebo_loop/GazeboGpsSubscribe/Enabled Subsystem'
//  '<S9>'   : 'FlightCore_Gazebo_loop/GazeboImuSubscribe/Enabled Subsystem'

#endif                                 // FlightCore_Gazebo_loop_h_

//
// File trailer for generated code.
//
// [EOF]
//
