//
// File: FlightCore_Gazebo_loop_types.h
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
#ifndef FlightCore_Gazebo_loop_types_h_
#define FlightCore_Gazebo_loop_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_SL_Bus_flightcore_gazebo_msgs_ActuatorCommand_
#define DEFINED_TYPEDEF_FOR_SL_Bus_flightcore_gazebo_msgs_ActuatorCommand_

// MsgType=flightcore_gazebo_msgs/ActuatorCommand
struct SL_Bus_flightcore_gazebo_msgs_ActuatorCommand
{
  uint32_T schema_version;
  uint64_T session_id;
  uint64_T source_step_id;
  uint64_T target_step_id;
  uint64_T command_id;
  uint64_T valid_from_iteration;
  boolean_T armed;
  boolean_T valid;
  real32_T actuator_values[4];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_SL_Bus_builtin_interfaces_Time_
#define DEFINED_TYPEDEF_FOR_SL_Bus_builtin_interfaces_Time_

// MsgType=builtin_interfaces/Time
struct SL_Bus_builtin_interfaces_Time
{
  int32_T sec;
  uint32_T nanosec;
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_SL_Bus_flightcore_msgs_Gps_
#define DEFINED_TYPEDEF_FOR_SL_Bus_flightcore_msgs_Gps_

// MsgType=flightcore_msgs/Gps
struct SL_Bus_flightcore_msgs_Gps
{
  // MsgType=builtin_interfaces/Time
  SL_Bus_builtin_interfaces_Time stamp;
  real_T timestamp_sec;
  uint32_T sequence;
  uint8_T source_id;
  boolean_T valid;
  real32_T lat_deg;
  real32_T lon_deg;
  real32_T alt_m;
  real32_T velocity_ned_mps[3];
};

#endif

#ifndef DEFINED_TYPEDEF_FOR_SL_Bus_flightcore_msgs_Imu_
#define DEFINED_TYPEDEF_FOR_SL_Bus_flightcore_msgs_Imu_

// MsgType=flightcore_msgs/Imu
struct SL_Bus_flightcore_msgs_Imu
{
  // MsgType=builtin_interfaces/Time
  SL_Bus_builtin_interfaces_Time stamp;
  real_T timestamp_sec;
  uint32_T sequence;
  uint8_T source_id;
  boolean_T valid;
  real32_T accel_mps2[3];
  real32_T gyro_radps[3];
};

#endif

// Custom Type definition for MATLABSystem: '<S5>/SourceBlock'
#include "rmw/qos_profiles.h"
#ifndef struct_sJ4ih70VmKcvCeguWN0mNVF
#define struct_sJ4ih70VmKcvCeguWN0mNVF

struct sJ4ih70VmKcvCeguWN0mNVF
{
  real_T sec;
  real_T nsec;
};

#endif                                 // struct_sJ4ih70VmKcvCeguWN0mNVF

#ifndef struct_ros_slros2_internal_block_Pub_T
#define struct_ros_slros2_internal_block_Pub_T

struct ros_slros2_internal_block_Pub_T
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T QOSAvoidROSNamespaceConventions;
};

#endif                                // struct_ros_slros2_internal_block_Pub_T

#ifndef struct_ros_slros2_internal_block_Sub_T
#define struct_ros_slros2_internal_block_Sub_T

struct ros_slros2_internal_block_Sub_T
{
  boolean_T matlabCodegenIsDeleted;
  int32_T isInitialized;
  boolean_T isSetupComplete;
  boolean_T QOSAvoidROSNamespaceConventions;
};

#endif                                // struct_ros_slros2_internal_block_Sub_T

// Forward declaration for rtModel
typedef struct tag_RTM_FlightCore_Gazebo_loo_T RT_MODEL_FlightCore_Gazebo_lo_T;

#endif                                 // FlightCore_Gazebo_loop_types_h_

//
// File trailer for generated code.
//
// [EOF]
//
