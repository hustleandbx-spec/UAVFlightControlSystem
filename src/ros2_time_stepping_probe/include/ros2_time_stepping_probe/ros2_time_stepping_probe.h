//
// File: ros2_time_stepping_probe.h
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
#ifndef ros2_time_stepping_probe_h_
#define ros2_time_stepping_probe_h_
#include "rtwtypes.h"
#include "slros2_initialize.h"
#include "ros2_time_stepping_probe_types.h"
#include <stddef.h>

// Block signals (default storage)
struct B_ros2_time_stepping_probe_T {
  char_T b_zeroDelimTopic[29];
};

// Block states (default storage) for system '<Root>'
struct DW_ros2_time_stepping_probe_T {
  ros_slros2_internal_block_Pub_T obj; // '<S2>/SinkBlock'
  uint64_T ExecutionCount_DSTATE;      // '<Root>/ExecutionCount'
};

// Real-time Model Data Structure
struct tag_RTM_ros2_time_stepping_pr_T {
  const char_T * volatile errorStatus;
  const char_T* getErrorStatus() const;
  void setErrorStatus(const char_T* const volatile aErrorStatus);
};

// Class declaration for model ros2_time_stepping_probe
class ros2_time_stepping_probe
{
  // public data and function members
 public:
  // Real-Time Model get method
  RT_MODEL_ros2_time_stepping_p_T * getRTM();

  // model initialize function
  void initialize();

  // model step function
  void step();

  // model terminate function
  void terminate();

  // Constructor
  ros2_time_stepping_probe();

  // Destructor
  ~ros2_time_stepping_probe();

  // private data and function members
 private:
  // Block signals
  B_ros2_time_stepping_probe_T ros2_time_stepping_probe_B;

  // Block states
  DW_ros2_time_stepping_probe_T ros2_time_stepping_probe_DW;

  // private member function(s) for subsystem '<Root>'
  void ros2_time_s_Publisher_setupImpl(const ros_slros2_internal_block_Pub_T
    *obj);

  // Real-Time Model
  RT_MODEL_ros2_time_stepping_p_T ros2_time_stepping_probe_M;
};

extern volatile boolean_T stopRequested;
extern volatile boolean_T runModel;

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
//  '<Root>' : 'ros2_time_stepping_probe'
//  '<S1>'   : 'ros2_time_stepping_probe/CountBlankMessage'
//  '<S2>'   : 'ros2_time_stepping_probe/CountPublish'

#endif                                 // ros2_time_stepping_probe_h_

//
// File trailer for generated code.
//
// [EOF]
//
