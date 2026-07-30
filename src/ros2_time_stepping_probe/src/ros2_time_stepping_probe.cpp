//
// File: ros2_time_stepping_probe.cpp
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
#include "ros2_time_stepping_probe.h"
#include "ros2_time_stepping_probe_types.h"
#include "rmw/qos_profiles.h"
#include "rtwtypes.h"
#include <stddef.h>

void ros2_time_stepping_probe::ros2_time_s_Publisher_setupImpl(const
  ros_slros2_internal_block_Pub_T *obj)
{
  rmw_qos_profile_t qos_profile;
  sJ4ih70VmKcvCeguWN0mNVF deadline;
  sJ4ih70VmKcvCeguWN0mNVF lifespan;
  sJ4ih70VmKcvCeguWN0mNVF liveliness_lease_duration;
  static const char_T b_zeroDelimTopic[29] = "/flightcore/time_probe/count";
  qos_profile = rmw_qos_profile_default;

  // Start for MATLABSystem: '<S2>/SinkBlock'
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
  for (int32_T i = 0; i < 29; i++) {
    // Start for MATLABSystem: '<S2>/SinkBlock'
    ros2_time_stepping_probe_B.b_zeroDelimTopic[i] = b_zeroDelimTopic[i];
  }

  Pub_ros2_time_stepping_probe_6.createPublisher
    (&ros2_time_stepping_probe_B.b_zeroDelimTopic[0], qos_profile);
}

// Model step function
void ros2_time_stepping_probe::step()
{
  SL_Bus_std_msgs_UInt64 rtb_AssignCount;

  // BusAssignment: '<Root>/AssignCount' incorporates:
  //   UnitDelay: '<Root>/ExecutionCount'

  rtb_AssignCount.data = ros2_time_stepping_probe_DW.ExecutionCount_DSTATE;

  // MATLABSystem: '<S2>/SinkBlock' incorporates:
  //   BusAssignment: '<Root>/AssignCount'

  Pub_ros2_time_stepping_probe_6.publish(&rtb_AssignCount);

  // Sum: '<Root>/Increment' incorporates:
  //   Constant: '<Root>/One'
  //   UnitDelay: '<Root>/ExecutionCount'

  ros2_time_stepping_probe_DW.ExecutionCount_DSTATE++;
}

// Model initialize function
void ros2_time_stepping_probe::initialize()
{
  // Start for MATLABSystem: '<S2>/SinkBlock'
  ros2_time_stepping_probe_DW.obj.QOSAvoidROSNamespaceConventions = false;
  ros2_time_stepping_probe_DW.obj.matlabCodegenIsDeleted = false;
  ros2_time_stepping_probe_DW.obj.isSetupComplete = false;
  ros2_time_stepping_probe_DW.obj.isInitialized = 1;
  ros2_time_s_Publisher_setupImpl(&ros2_time_stepping_probe_DW.obj);
  ros2_time_stepping_probe_DW.obj.isSetupComplete = true;
}

// Model terminate function
void ros2_time_stepping_probe::terminate()
{
  // Terminate for MATLABSystem: '<S2>/SinkBlock'
  if (!ros2_time_stepping_probe_DW.obj.matlabCodegenIsDeleted) {
    ros2_time_stepping_probe_DW.obj.matlabCodegenIsDeleted = true;
    if ((ros2_time_stepping_probe_DW.obj.isInitialized == 1) &&
        ros2_time_stepping_probe_DW.obj.isSetupComplete) {
      Pub_ros2_time_stepping_probe_6.resetPublisherPtr();//();
    }
  }

  // End of Terminate for MATLABSystem: '<S2>/SinkBlock'
}

// Constructor
ros2_time_stepping_probe::ros2_time_stepping_probe() :
  ros2_time_stepping_probe_B(),
  ros2_time_stepping_probe_DW(),
  ros2_time_stepping_probe_M()
{
  // Currently there is no constructor body generated.
}

// Destructor
ros2_time_stepping_probe::~ros2_time_stepping_probe()
{
  // Currently there is no destructor body generated.
}

// Real-Time Model get method
RT_MODEL_ros2_time_stepping_p_T * ros2_time_stepping_probe::getRTM()
{
  return (&ros2_time_stepping_probe_M);
}

const char_T* RT_MODEL_ros2_time_stepping_p_T::getErrorStatus() const
{
  return (errorStatus);
}

void RT_MODEL_ros2_time_stepping_p_T::setErrorStatus(const char_T* const
  volatile aErrorStatus)
{
  (errorStatus = aErrorStatus);
}

//
// File trailer for generated code.
//
// [EOF]
//
