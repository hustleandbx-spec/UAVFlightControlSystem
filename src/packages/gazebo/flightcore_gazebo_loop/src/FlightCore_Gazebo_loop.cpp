//
// File: FlightCore_Gazebo_loop.cpp
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
#include "FlightCore_Gazebo_loop.h"
#include "FlightCore_Gazebo_loop_types.h"
#include <cstring>
#include "rmw/qos_profiles.h"
#include "rtwtypes.h"
#include <stddef.h>
#include "FlightCore.h"

// Exported block parameters
real32_T R_pos{ 0.2F };                // Variable: R_pos
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  GPS 位置观测噪声标准差。增大→降低 GPS 位置在修正中的权重


real32_T R_vel{ 0.1F };                // Variable: R_vel
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  GPS 速度观测噪声标准差。增大→降低 GPS 速度在修正中的权重


real32_T SE_EKF_INIT_STATE[16]{ 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F,
  0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F, 0.0F } ;// Variable: SE_EKF_INIT_STATE
                                                       //  Referenced by: '<Root>/FlightCore'
                                                       //  EKF 16维默认名义状态 [Position_NED; Velocity_NED; Attitude_quat_wxyz; AccelBias; GyroBias]。位置以启动点为局部 NED 原点。


real32_T attP{ 8.0F };                 // Variable: attP
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  姿态 P 增益。姿态误差→角速率指令。只用P，I由角速率环承担，防止两级积分器串联振荡


real32_T ekf_predict_dt{ 0.001F };     // Variable: ekf_predict_dt
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  EKF 预测步长数值输入。显式匹配 EKF MATLAB Function 的 single 数值契约


real32_T g_n[3]{ 0.0F, 0.0F, 9.81F } ; // Variable: g_n
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  NED 重力加速度矢量 [g_N, g_E, g_D]。物理常量，不作调参项


real32_T mass{ 1.0F };                 // Variable: mass
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  AirSim Generic F450 四旋翼总质量（含电池、载荷）。来源: MultiRotorParams::setupFrameGenericQuad


real32_T maxAcc_xy{ 5.0F };            // Variable: maxAcc_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平加速度指令上限。限制最大姿态倾斜角


real32_T maxAcc_z{ 10.0F };            // Variable: maxAcc_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直加速度指令上限


real32_T maxRate{ 3.1416F };           // Variable: maxRate
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  最大角速率指令 (= π rad/s ≈ 180°/s)


real32_T maxTorque{ 2.0F };            // Variable: maxTorque
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  单轴力矩指令上限。在电机饱和前拦截过大指令，确保控制在线性区


real32_T maxVel_xy{ 10.0F };           // Variable: maxVel_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平速度指令上限


real32_T maxVel_z{ 5.0F };             // Variable: maxVel_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直速度指令上限


real32_T mixMatrix[16]{ 1.0F, 1.0F, 1.0F, 1.0F, -1.0F, 1.0F, 1.0F, -1.0F, -1.0F,
  1.0F, -1.0F, 1.0F, -1.0F, -1.0F, 1.0F, 1.0F } ;// Variable: mixMatrix
                                                    //  Referenced by: '<Root>/FlightCore'
                                                    //  X型四旋翼混控矩阵 (4×4)。[F, τx, τy, τz]&#x1D40; → [m1, m2, m3, m4]&#x1D40;。列分别对应: 总推力、滚转力矩、俯仰力矩、偏航力矩


real32_T motorArmLength_m{ 0.2275F };  // Variable: motorArmLength_m
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  AirSim Generic F450 电机到重心的水平距离（臂长）


real32_T motorMax{ 1.0F };             // Variable: motorMax
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  电机指令最大值（归一化 0~1）。硬件保护上限


real32_T motorMaxReactionTorque_Nm{ 0.055562F };// Variable: motorMaxReactionTorque_Nm
                                                   //  Referenced by: '<Root>/FlightCore'
                                                   //  AirSim Generic Quad 单旋翼最大反扭矩。normalized command 1.0 线性对应此反扭矩


real32_T motorMaxThrust_N{ 4.17944622F };// Variable: motorMaxThrust_N
                                            //  Referenced by: '<Root>/FlightCore'
                                            //  AirSim Generic Quad 单旋翼最大推力。normalized command 1.0 线性对应此推力


real32_T motorMin{ 0.05F };            // Variable: motorMin
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  电机指令最小值（归一化 0~1）。>0 防止电机停转失去姿态调控能力


real32_T posD_xy{ 0.0F };              // Variable: posD_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平位置 D 增益。当前=0（微分由角速率环 D 提供）


real32_T posD_z{ 0.0F };               // Variable: posD_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  高度位置 D 增益。当前=0


real32_T posFilterN{ 20.0F };          // Variable: posFilterN
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  位置PID导数滤波系数；在50 Hz周期下满足N*Ts不超过0.5。


real32_T posI_xy{ 0.02F };             // Variable: posI_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平位置 I 增益。消除悬停稳态位置误差和常值风扰


real32_T posI_z{ 0.1F };               // Variable: posI_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  高度位置 I 增益


real32_T posP_xy{ 1.0F };              // Variable: posP_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平位置 P 增益。位置误差→速度指令。增大加快位置响应，过大引发超调


real32_T posP_z{ 2.0F };               // Variable: posP_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  高度位置 P 增益。垂直增益 > 水平增益以积极对抗重力偏差


real32_T rateD{ 0.005F };              // Variable: rateD
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  角速率 D 增益。提供角速率阻尼，抑制姿态修正时的振荡


real32_T rateFilterN{ 100.0F };        // Variable: rateFilterN
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  角速率PID导数滤波器系数。


real32_T rateI{ 0.02F };               // Variable: rateI
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  角速率 I 增益。消除角速率稳态误差、补偿常值扰动力矩


real32_T rateP{ 0.15F };               // Variable: rateP
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  角速率 P 增益。对抗角速率扰动（突风、重心偏移）


real32_T sigma_acc{ 0.001F };          // Variable: sigma_acc
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  加速度计白噪声 PSD。增大→降低加速度计在融合中的权重


real32_T sigma_ba{ 1.0E-6F };          // Variable: sigma_ba
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  加速度计偏置随机游走 PSD。增大→允许偏置估计更快变化但噪声更大


real32_T sigma_bg{ 1.0E-7F };          // Variable: sigma_bg
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  陀螺仪偏置随机游走 PSD。增大→允许偏置估计更快变化但噪声更大


real32_T sigma_gyr{ 0.0001F };         // Variable: sigma_gyr
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  陀螺仪白噪声 PSD。增大→降低陀螺仪在融合中的权重


real32_T velD_xy{ 0.0F };              // Variable: velD_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平速度 D 增益。当前=0


real32_T velD_z{ 0.0F };               // Variable: velD_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直速度 D 增益。当前=0


real32_T velFilterN{ 20.0F };          // Variable: velFilterN
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  速度PID导数滤波系数；在50 Hz周期下满足N*Ts不超过0.5。


real32_T velI_xy{ 0.02F };             // Variable: velI_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平速度 I 增益。消除速度稳态误差


real32_T velI_z{ 0.05F };              // Variable: velI_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直速度 I 增益


real32_T velP_xy{ 0.15F };             // Variable: velP_xy
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  水平速度 P 增益。速度误差→加速度指令


real32_T velP_z{ 1.0F };               // Variable: velP_z
                                          //  Referenced by: '<Root>/FlightCore'
                                          //  垂直速度 P 增益


static void rate_scheduler(RT_MODEL_FlightCore_Gazebo_lo_T *const
  FlightCore_Gazebo_loop_M);

//
//         This function updates active task flag for each subrate.
//         The function is called at model base rate, hence the
//         generated code self-manages all its subrates.
//
static void rate_scheduler(RT_MODEL_FlightCore_Gazebo_lo_T *const
  FlightCore_Gazebo_loop_M)
{
  // Compute which subrates run during the next base time step.  Subrates
  //  are an integer multiple of the base rate counter.  Therefore, the subtask
  //  counter is reset when it reaches its limit (zero means run).

  (FlightCore_Gazebo_loop_M->Timing.TaskCounters.TID[2])++;
  if ((FlightCore_Gazebo_loop_M->Timing.TaskCounters.TID[2]) > 3) {// Sample time: [0.004s, 0.0s] 
    FlightCore_Gazebo_loop_M->Timing.TaskCounters.TID[2] = 0;
  }

  (FlightCore_Gazebo_loop_M->Timing.TaskCounters.TID[3])++;
  if ((FlightCore_Gazebo_loop_M->Timing.TaskCounters.TID[3]) > 19) {// Sample time: [0.02s, 0.0s] 
    FlightCore_Gazebo_loop_M->Timing.TaskCounters.TID[3] = 0;
  }
}

void FlightCore_Gazebo_loop::FlightCo_Subscriber_setupImpl_g(const
  ros_slros2_internal_block_Sub_T *obj)
{
  static const char_T b_zeroDelimTopic[23]{ "/flightcore/gazebo/imu" };

  rmw_qos_profile_t qos_profile;
  sJ4ih70VmKcvCeguWN0mNVF deadline;
  sJ4ih70VmKcvCeguWN0mNVF lifespan;
  sJ4ih70VmKcvCeguWN0mNVF liveliness_lease_duration;
  qos_profile = rmw_qos_profile_default;

  // Start for MATLABSystem: '<S5>/SourceBlock'
  deadline.sec = 0.0;
  deadline.nsec = 0.0;
  lifespan.sec = 0.0;
  lifespan.nsec = 0.0;
  liveliness_lease_duration.sec = 0.0;
  liveliness_lease_duration.nsec = 0.0;
  SET_QOS_VALUES(qos_profile, RMW_QOS_POLICY_HISTORY_KEEP_LAST, (size_t)10.0,
                 RMW_QOS_POLICY_DURABILITY_VOLATILE,
                 RMW_QOS_POLICY_RELIABILITY_RELIABLE, deadline, lifespan,
                 RMW_QOS_POLICY_LIVELINESS_AUTOMATIC, liveliness_lease_duration,
                 (bool)obj->QOSAvoidROSNamespaceConventions);
  for (int32_T i{0}; i < 23; i++) {
    // Start for MATLABSystem: '<S5>/SourceBlock'
    FlightCore_Gazebo_loop_B.b_zeroDelimTopic_c[i] = b_zeroDelimTopic[i];
  }

  Sub_FlightCore_Gazebo_loop_187.createSubscriber
    (&FlightCore_Gazebo_loop_B.b_zeroDelimTopic_c[0], qos_profile);
}

void FlightCore_Gazebo_loop::FlightCore_Subscriber_setupImpl(const
  ros_slros2_internal_block_Sub_T *obj)
{
  static const char_T b_zeroDelimTopic[23]{ "/flightcore/gazebo/gps" };

  rmw_qos_profile_t qos_profile;
  sJ4ih70VmKcvCeguWN0mNVF deadline;
  sJ4ih70VmKcvCeguWN0mNVF lifespan;
  sJ4ih70VmKcvCeguWN0mNVF liveliness_lease_duration;
  qos_profile = rmw_qos_profile_default;

  // Start for MATLABSystem: '<S3>/SourceBlock'
  deadline.sec = 0.0;
  deadline.nsec = 0.0;
  lifespan.sec = 0.0;
  lifespan.nsec = 0.0;
  liveliness_lease_duration.sec = 0.0;
  liveliness_lease_duration.nsec = 0.0;
  SET_QOS_VALUES(qos_profile, RMW_QOS_POLICY_HISTORY_KEEP_LAST, (size_t)10.0,
                 RMW_QOS_POLICY_DURABILITY_VOLATILE,
                 RMW_QOS_POLICY_RELIABILITY_RELIABLE, deadline, lifespan,
                 RMW_QOS_POLICY_LIVELINESS_AUTOMATIC, liveliness_lease_duration,
                 (bool)obj->QOSAvoidROSNamespaceConventions);
  for (int32_T i{0}; i < 23; i++) {
    // Start for MATLABSystem: '<S3>/SourceBlock'
    FlightCore_Gazebo_loop_B.b_zeroDelimTopic_k[i] = b_zeroDelimTopic[i];
  }

  Sub_FlightCore_Gazebo_loop_189.createSubscriber
    (&FlightCore_Gazebo_loop_B.b_zeroDelimTopic_k[0], qos_profile);
}

void FlightCore_Gazebo_loop::FlightCore__Publisher_setupImpl(const
  ros_slros2_internal_block_Pub_T *obj)
{
  static const char_T b_zeroDelimTopic[36]{
    "/flightcore/gazebo/actuator_command" };

  rmw_qos_profile_t qos_profile;
  sJ4ih70VmKcvCeguWN0mNVF deadline;
  sJ4ih70VmKcvCeguWN0mNVF lifespan;
  sJ4ih70VmKcvCeguWN0mNVF liveliness_lease_duration;
  qos_profile = rmw_qos_profile_default;

  // Start for MATLABSystem: '<S7>/SinkBlock'
  deadline.sec = 0.0;
  deadline.nsec = 0.0;
  lifespan.sec = 0.0;
  lifespan.nsec = 0.0;
  liveliness_lease_duration.sec = 0.0;
  liveliness_lease_duration.nsec = 0.0;
  SET_QOS_VALUES(qos_profile, RMW_QOS_POLICY_HISTORY_KEEP_LAST, (size_t)1.0,
                 RMW_QOS_POLICY_DURABILITY_VOLATILE,
                 RMW_QOS_POLICY_RELIABILITY_RELIABLE, deadline, lifespan,
                 RMW_QOS_POLICY_LIVELINESS_AUTOMATIC, liveliness_lease_duration,
                 (bool)obj->QOSAvoidROSNamespaceConventions);
  for (int32_T i{0}; i < 36; i++) {
    // Start for MATLABSystem: '<S7>/SinkBlock'
    FlightCore_Gazebo_loop_B.b_zeroDelimTopic[i] = b_zeroDelimTopic[i];
  }

  Pub_FlightCore_Gazebo_loop_42.createPublisher
    (&FlightCore_Gazebo_loop_B.b_zeroDelimTopic[0], qos_profile);
}

// Model step function
void FlightCore_Gazebo_loop::step()
{
  // BusAssignment: '<S1>/ActuatorCommandAssign'
  std::memset(&FlightCore_Gazebo_loop_B.ActuatorCommandAssign, 0, sizeof
              (SL_Bus_flightcore_gazebo_msgs_ActuatorCommand));

  // MATLABSystem: '<S5>/SourceBlock'
  FlightCore_Gazebo_loop_B.SourceBlock_o1 =
    Sub_FlightCore_Gazebo_loop_187.getLatestMessage
    (&FlightCore_Gazebo_loop_B.rtb_SourceBlock_o2_c);

  // Outputs for Enabled SubSystem: '<S5>/Enabled Subsystem' incorporates:
  //   EnablePort: '<S9>/Enable'

  if (FlightCore_Gazebo_loop_B.SourceBlock_o1) {
    // SignalConversion generated from: '<S9>/In1' incorporates:
    //   MATLABSystem: '<S5>/SourceBlock'

    FlightCore_Gazebo_loop_B.In1 = FlightCore_Gazebo_loop_B.rtb_SourceBlock_o2_c;
  }

  // End of Outputs for SubSystem: '<S5>/Enabled Subsystem'

  // MATLABSystem: '<S3>/SourceBlock'
  FlightCore_Gazebo_loop_B.SourceBlock_o1_c =
    Sub_FlightCore_Gazebo_loop_189.getLatestMessage
    (&FlightCore_Gazebo_loop_B.rtb_SourceBlock_o2_n_m);

  // Outputs for Enabled SubSystem: '<S3>/Enabled Subsystem' incorporates:
  //   EnablePort: '<S8>/Enable'

  if (FlightCore_Gazebo_loop_B.SourceBlock_o1_c) {
    // SignalConversion generated from: '<S8>/In1' incorporates:
    //   MATLABSystem: '<S3>/SourceBlock'

    FlightCore_Gazebo_loop_B.In1_n =
      FlightCore_Gazebo_loop_B.rtb_SourceBlock_o2_n_m;
  }

  // End of Outputs for SubSystem: '<S3>/Enabled Subsystem'

  // ModelReference generated from: '<Root>/FlightCore'
  FlightCore(&(FlightCore_Gazebo_loop_DW.FlightCore_InstanceData.rtm),
             &FlightCore_Gazebo_loop_B.In1.accel_mps2[0],
             &FlightCore_Gazebo_loop_B.In1.gyro_radps[0],
             &FlightCore_Gazebo_loop_B.In1.valid,
             &FlightCore_Gazebo_loop_B.In1_n.lat_deg,
             &FlightCore_Gazebo_loop_B.In1_n.lon_deg,
             &FlightCore_Gazebo_loop_B.In1_n.alt_m,
             &FlightCore_Gazebo_loop_B.In1_n.velocity_ned_mps[0],
             &FlightCore_Gazebo_loop_B.In1_n.valid,
             &FlightCore_Gazebo_loop_B.SourceBlock_o1_c,
             &FlightCore_Gazebo_loop_B.MotorCmd[0],
             &FlightCore_Gazebo_loop_B.Armed, &FlightCore_Gazebo_loop_B.Valid,
             &FlightCore_Gazebo_loop_B.Position_NED[0],
             &FlightCore_Gazebo_loop_B.Velocity_NED[0],
             &FlightCore_Gazebo_loop_B.Attitude_quat[0],
             &FlightCore_Gazebo_loop_B.AngularRate_Body[0],
             &FlightCore_Gazebo_loop_B.Accel_Body[0],
             &FlightCore_Gazebo_loop_B.GyroBias[0],
             &FlightCore_Gazebo_loop_B.AccelBias[0],
             &FlightCore_Gazebo_loop_B.Wind_NED[0],
             &FlightCore_Gazebo_loop_B.Status,
             &(FlightCore_Gazebo_loop_DW.FlightCore_InstanceData.rtb),
             &(FlightCore_Gazebo_loop_DW.FlightCore_InstanceData.rtdw));

  // Bias: '<S1>/NextStepId' incorporates:
  //   UnitDelay: '<Root>/ExecutionCount'

  FlightCore_Gazebo_loop_B.qY = FlightCore_Gazebo_loop_DW.ExecutionCount_DSTATE
    + /*MW:OvSatOk*/ 1UL;
  if (FlightCore_Gazebo_loop_DW.ExecutionCount_DSTATE + 1UL <
      FlightCore_Gazebo_loop_DW.ExecutionCount_DSTATE) {
    FlightCore_Gazebo_loop_B.qY = MAX_uint64_T;
  }

  // BusAssignment: '<S1>/ActuatorCommandAssign' incorporates:
  //   Bias: '<S1>/NextIteration'
  //   Bias: '<S1>/NextStepId'
  //   UnitDelay: '<Root>/ExecutionCount'

  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.source_step_id =
    FlightCore_Gazebo_loop_DW.ExecutionCount_DSTATE;
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.target_step_id =
    FlightCore_Gazebo_loop_B.qY;
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.command_id =
    FlightCore_Gazebo_loop_B.qY;
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.valid_from_iteration =
    FlightCore_Gazebo_loop_DW.ExecutionCount_DSTATE + /*MW:OvSatOk*/ 1UL;

  // Bias: '<S1>/NextIteration' incorporates:
  //   UnitDelay: '<Root>/ExecutionCount'

  if (FlightCore_Gazebo_loop_DW.ExecutionCount_DSTATE + 1UL <
      FlightCore_Gazebo_loop_DW.ExecutionCount_DSTATE) {
    // BusAssignment: '<S1>/ActuatorCommandAssign'
    FlightCore_Gazebo_loop_B.ActuatorCommandAssign.valid_from_iteration =
      MAX_uint64_T;
  }

  // BusAssignment: '<S1>/ActuatorCommandAssign'
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.armed =
    FlightCore_Gazebo_loop_B.Armed;
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.valid =
    FlightCore_Gazebo_loop_B.Valid;
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.actuator_values[0] =
    FlightCore_Gazebo_loop_B.MotorCmd[0];
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.actuator_values[1] =
    FlightCore_Gazebo_loop_B.MotorCmd[1];
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.actuator_values[2] =
    FlightCore_Gazebo_loop_B.MotorCmd[2];
  FlightCore_Gazebo_loop_B.ActuatorCommandAssign.actuator_values[3] =
    FlightCore_Gazebo_loop_B.MotorCmd[3];

  // MATLABSystem: '<S7>/SinkBlock' incorporates:
  //   BusAssignment: '<S1>/ActuatorCommandAssign'

  Pub_FlightCore_Gazebo_loop_42.publish
    (&FlightCore_Gazebo_loop_B.ActuatorCommandAssign);

  // Sum: '<Root>/IncrementExecutionCount' incorporates:
  //   Constant: '<Root>/ExecutionCountIncrement'
  //   UnitDelay: '<Root>/ExecutionCount'

  FlightCore_Gazebo_loop_DW.ExecutionCount_DSTATE++;

  // Update absolute time for base rate
  // The "clockTick0" counts the number of times the code of this task has
  //  been executed. The absolute time is the multiplication of "clockTick0"
  //  and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
  //  overflow during the application lifespan selected.

  (&FlightCore_Gazebo_loop_M)->Timing.t[0] =
    ((time_T)(++(&FlightCore_Gazebo_loop_M)->Timing.clockTick0)) *
    (&FlightCore_Gazebo_loop_M)->Timing.stepSize0;

  {
    // Update absolute timer for sample time: [0.001s, 0.0s]
    // The "clockTick1" counts the number of times the code of this task has
    //  been executed. The resolution of this integer timer is 0.001, which is the step size
    //  of the task. Size of "clockTick1" ensures timer will not overflow during the
    //  application lifespan selected.

    (&FlightCore_Gazebo_loop_M)->Timing.clockTick1++;
  }

  if ((&FlightCore_Gazebo_loop_M)->Timing.TaskCounters.TID[2] == 0) {
    // Update absolute timer for sample time: [0.004s, 0.0s]
    // The "clockTick2" counts the number of times the code of this task has
    //  been executed. The resolution of this integer timer is 0.004, which is the step size
    //  of the task. Size of "clockTick2" ensures timer will not overflow during the
    //  application lifespan selected.

    (&FlightCore_Gazebo_loop_M)->Timing.clockTick2++;
  }

  if ((&FlightCore_Gazebo_loop_M)->Timing.TaskCounters.TID[3] == 0) {
    // Update absolute timer for sample time: [0.02s, 0.0s]
    // The "clockTick3" counts the number of times the code of this task has
    //  been executed. The resolution of this integer timer is 0.02, which is the step size
    //  of the task. Size of "clockTick3" ensures timer will not overflow during the
    //  application lifespan selected.

    (&FlightCore_Gazebo_loop_M)->Timing.clockTick3++;
  }

  rate_scheduler((&FlightCore_Gazebo_loop_M));
}

// Model initialize function
void FlightCore_Gazebo_loop::initialize()
{
  // Registration code
  {
    // Setup solver object
    rtsiSetSimTimeStepPtr(&(&FlightCore_Gazebo_loop_M)->solverInfo,
                          &(&FlightCore_Gazebo_loop_M)->Timing.simTimeStep);
    rtsiSetTPtr(&(&FlightCore_Gazebo_loop_M)->solverInfo,
                (&FlightCore_Gazebo_loop_M)->getTPtrPtr());
    rtsiSetStepSizePtr(&(&FlightCore_Gazebo_loop_M)->solverInfo,
                       &(&FlightCore_Gazebo_loop_M)->Timing.stepSize0);
    rtsiSetErrorStatusPtr(&(&FlightCore_Gazebo_loop_M)->solverInfo,
                          (&FlightCore_Gazebo_loop_M)->getErrorStatusPtr());
    rtsiSetRTModelPtr(&(&FlightCore_Gazebo_loop_M)->solverInfo,
                      (&FlightCore_Gazebo_loop_M));
  }

  rtsiSetSimTimeStep(&(&FlightCore_Gazebo_loop_M)->solverInfo, MAJOR_TIME_STEP);
  rtsiSetIsMinorTimeStepWithModeChange(&(&FlightCore_Gazebo_loop_M)->solverInfo,
    false);
  rtsiSetIsContModeFrozen(&(&FlightCore_Gazebo_loop_M)->solverInfo, false);
  rtsiSetSolverName(&(&FlightCore_Gazebo_loop_M)->solverInfo,"FixedStepDiscrete");
  (&FlightCore_Gazebo_loop_M)->setTPtr(&(&FlightCore_Gazebo_loop_M)
    ->Timing.tArray[0]);
  (&FlightCore_Gazebo_loop_M)->Timing.stepSize0 = 0.001;

  {
    static uint32_T *clockTickPtrs[4];
    static real_T *taskTimePtrs[4];
    static uint32_T *taskCounterPtrs;
    (&FlightCore_Gazebo_loop_M)->timingBridge.nTasks = 4;
    clockTickPtrs[0] = &((&FlightCore_Gazebo_loop_M)->Timing.clockTick0);
    clockTickPtrs[1] = &((&FlightCore_Gazebo_loop_M)->Timing.clockTick1);
    clockTickPtrs[2] = &((&FlightCore_Gazebo_loop_M)->Timing.clockTick2);
    clockTickPtrs[3] = &((&FlightCore_Gazebo_loop_M)->Timing.clockTick3);
    (&FlightCore_Gazebo_loop_M)->timingBridge.clockTick = clockTickPtrs;
    (&FlightCore_Gazebo_loop_M)->timingBridge.clockTickH = (nullptr);
    taskCounterPtrs = &((&FlightCore_Gazebo_loop_M)->Timing.TaskCounters.TID[0]);
    (&FlightCore_Gazebo_loop_M)->timingBridge.taskCounter = taskCounterPtrs;
    taskTimePtrs[0] = &((&FlightCore_Gazebo_loop_M)->Timing.t[0]);
    taskTimePtrs[1] = (nullptr);
    taskTimePtrs[2] = (nullptr);
    taskTimePtrs[3] = (nullptr);
    (&FlightCore_Gazebo_loop_M)->timingBridge.taskTime = taskTimePtrs;
  }

  // Model Initialize function for ModelReference Block: '<Root>/FlightCore'
  FlightCore_initialize((&FlightCore_Gazebo_loop_M)->getErrorStatusPointer(),
                        &((&FlightCore_Gazebo_loop_M)->solverInfo),
                        &(&FlightCore_Gazebo_loop_M)->timingBridge, 0, 1, 2, 3,
                        &(FlightCore_Gazebo_loop_DW.FlightCore_InstanceData.rtm),
                        &(FlightCore_Gazebo_loop_DW.FlightCore_InstanceData.rtdw),
                        &(FlightCore_Gazebo_loop_DW.FlightCore_InstanceData.rtzce));

  // SystemInitialize for ModelReference generated from: '<Root>/FlightCore'
  FlightCore_Init(&(FlightCore_Gazebo_loop_DW.FlightCore_InstanceData.rtdw));

  // Start for MATLABSystem: '<S5>/SourceBlock'
  FlightCore_Gazebo_loop_DW.obj_d.QOSAvoidROSNamespaceConventions = false;
  FlightCore_Gazebo_loop_DW.obj_d.matlabCodegenIsDeleted = false;
  FlightCore_Gazebo_loop_DW.obj_d.isSetupComplete = false;
  FlightCore_Gazebo_loop_DW.obj_d.isInitialized = 1;
  FlightCo_Subscriber_setupImpl_g(&FlightCore_Gazebo_loop_DW.obj_d);
  FlightCore_Gazebo_loop_DW.obj_d.isSetupComplete = true;

  // Start for MATLABSystem: '<S3>/SourceBlock'
  FlightCore_Gazebo_loop_DW.obj_dn.QOSAvoidROSNamespaceConventions = false;
  FlightCore_Gazebo_loop_DW.obj_dn.matlabCodegenIsDeleted = false;
  FlightCore_Gazebo_loop_DW.obj_dn.isSetupComplete = false;
  FlightCore_Gazebo_loop_DW.obj_dn.isInitialized = 1;
  FlightCore_Subscriber_setupImpl(&FlightCore_Gazebo_loop_DW.obj_dn);
  FlightCore_Gazebo_loop_DW.obj_dn.isSetupComplete = true;

  // Start for MATLABSystem: '<S7>/SinkBlock'
  FlightCore_Gazebo_loop_DW.obj.QOSAvoidROSNamespaceConventions = false;
  FlightCore_Gazebo_loop_DW.obj.matlabCodegenIsDeleted = false;
  FlightCore_Gazebo_loop_DW.obj.isSetupComplete = false;
  FlightCore_Gazebo_loop_DW.obj.isInitialized = 1;
  FlightCore__Publisher_setupImpl(&FlightCore_Gazebo_loop_DW.obj);
  FlightCore_Gazebo_loop_DW.obj.isSetupComplete = true;
}

// Model terminate function
void FlightCore_Gazebo_loop::terminate()
{
  // Terminate for MATLABSystem: '<S5>/SourceBlock'
  if (!FlightCore_Gazebo_loop_DW.obj_d.matlabCodegenIsDeleted) {
    FlightCore_Gazebo_loop_DW.obj_d.matlabCodegenIsDeleted = true;
    if ((FlightCore_Gazebo_loop_DW.obj_d.isInitialized == 1) &&
        FlightCore_Gazebo_loop_DW.obj_d.isSetupComplete) {
      Sub_FlightCore_Gazebo_loop_187.resetSubscriberPtr();//();
    }
  }

  // End of Terminate for MATLABSystem: '<S5>/SourceBlock'

  // Terminate for MATLABSystem: '<S3>/SourceBlock'
  if (!FlightCore_Gazebo_loop_DW.obj_dn.matlabCodegenIsDeleted) {
    FlightCore_Gazebo_loop_DW.obj_dn.matlabCodegenIsDeleted = true;
    if ((FlightCore_Gazebo_loop_DW.obj_dn.isInitialized == 1) &&
        FlightCore_Gazebo_loop_DW.obj_dn.isSetupComplete) {
      Sub_FlightCore_Gazebo_loop_189.resetSubscriberPtr();//();
    }
  }

  // End of Terminate for MATLABSystem: '<S3>/SourceBlock'

  // Terminate for MATLABSystem: '<S7>/SinkBlock'
  if (!FlightCore_Gazebo_loop_DW.obj.matlabCodegenIsDeleted) {
    FlightCore_Gazebo_loop_DW.obj.matlabCodegenIsDeleted = true;
    if ((FlightCore_Gazebo_loop_DW.obj.isInitialized == 1) &&
        FlightCore_Gazebo_loop_DW.obj.isSetupComplete) {
      Pub_FlightCore_Gazebo_loop_42.resetPublisherPtr();//();
    }
  }

  // End of Terminate for MATLABSystem: '<S7>/SinkBlock'
}

// Constructor
FlightCore_Gazebo_loop::FlightCore_Gazebo_loop() :
  FlightCore_Gazebo_loop_B(),
  FlightCore_Gazebo_loop_DW(),
  FlightCore_Gazebo_loop_M()
{
  // Currently there is no constructor body generated.
}

// Destructor
// Currently there is no destructor body generated.
FlightCore_Gazebo_loop::~FlightCore_Gazebo_loop() = default;

// Real-Time Model get method
RT_MODEL_FlightCore_Gazebo_lo_T * FlightCore_Gazebo_loop::getRTM()
{
  return (&FlightCore_Gazebo_loop_M);
}

time_T** RT_MODEL_FlightCore_Gazebo_lo_T::getTPtrPtr()
{
  return &(Timing.t);
}

const char_T* RT_MODEL_FlightCore_Gazebo_lo_T::getErrorStatus() const
{
  return (errorStatus);
}

void RT_MODEL_FlightCore_Gazebo_lo_T::setErrorStatus(const char_T* const
  aErrorStatus)
{
  (errorStatus = aErrorStatus);
}

time_T* RT_MODEL_FlightCore_Gazebo_lo_T::getTPtr() const
{
  return (Timing.t);
}

void RT_MODEL_FlightCore_Gazebo_lo_T::setTPtr(time_T* aTPtr)
{
  (Timing.t = aTPtr);
}

const char_T** RT_MODEL_FlightCore_Gazebo_lo_T::getErrorStatusPtr()
{
  return &errorStatus;
}

const char_T** RT_MODEL_FlightCore_Gazebo_lo_T::getErrorStatusPointer()
{
  return &errorStatus;
}

boolean_T RT_MODEL_FlightCore_Gazebo_lo_T::isMajorTimeStep() const
{
  return ((Timing.simTimeStep) == MAJOR_TIME_STEP);
}

boolean_T RT_MODEL_FlightCore_Gazebo_lo_T::isMinorTimeStep() const
{
  return ((Timing.simTimeStep) == MINOR_TIME_STEP);
}

//
// File trailer for generated code.
//
// [EOF]
//
