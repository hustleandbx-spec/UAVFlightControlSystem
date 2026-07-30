//
// File: ros2_time_stepping_probe_types.h
//
// Code generated for Simulink model 'ros2_time_stepping_probe'.
//
// Model version                  : 1.2
// Simulink Coder version         : 25.2 (R2025b) 28-Jul-2025
// C/C++ source code generated on : Tue Jul 28 16:51:06 2026
//
// Target selection: ert.tlc
// Embedded hardware selection: Intel->x86-64 (Linux 64)
// Code generation objectives: Unspecified
// Validation result: Not run
//
#ifndef ros2_time_stepping_probe_types_h_
#define ros2_time_stepping_probe_types_h_
#include "rtwtypes.h"
#ifndef DEFINED_TYPEDEF_FOR_SL_Bus_std_msgs_UInt64_
#define DEFINED_TYPEDEF_FOR_SL_Bus_std_msgs_UInt64_

// MsgType=std_msgs/UInt64
struct SL_Bus_std_msgs_UInt64
{
  uint64_T data;
};

#endif

// Custom Type definition for MATLABSystem: '<S2>/SinkBlock'
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

// Forward declaration for rtModel
typedef struct tag_RTM_ros2_time_stepping_pr_T RT_MODEL_ros2_time_stepping_p_T;

#endif                                 // ros2_time_stepping_probe_types_h_

//
// File trailer for generated code.
//
// [EOF]
//
