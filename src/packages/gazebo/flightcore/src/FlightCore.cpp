//
// File: FlightCore.cpp
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
#include "FlightCore.h"
#include "rtwtypes.h"
#include "EKF.h"
#include "UAV_FlightControl.h"

// Named constants for Chart: '<S2>/Chart'
const uint8_T FlightCore_IN_ACTIVE{ 1U };

const uint8_T FlightCore_IN_CONTROL_INHIBITED{ 2U };

const uint8_T FlightCore_IN_DISARMED{ 3U };

static boolean_T resultZC0;

// System initialize for referenced model: 'FlightCore'
void FlightCore_Init(DW_FlightCore_f_T *localDW)
{
  // SystemInitialize for ModelReference generated from: '<Root>/EKF'
  EKF_Init(&(localDW->EKF_InstanceData.rtdw));

  // SystemInitialize for Chart: '<S2>/Chart'
  localDW->previousZC = 3U;

  // SystemInitialize for ModelReference generated from: '<Root>/UAV_FlightControl' 
  UAV_FlightControl_Init(&(localDW->UAV_FlightControl_InstanceData.rtdw));
}

// Disable for referenced model: 'FlightCore'
void FlightCore_Disable(DW_FlightCore_f_T *localDW)
{
  // Disable for ModelReference generated from: '<Root>/UAV_FlightControl'
  UAV_FlightControl_Disable(&(localDW->UAV_FlightControl_InstanceData.rtb),
    &(localDW->UAV_FlightControl_InstanceData.rtdw));
}

// Output and update for referenced model: 'FlightCore'
void FlightCore(RT_MODEL_FlightCore_T * const FlightCore_M, const real32_T
                rtu_IMU_BUS_Accel[3], const real32_T rtu_IMU_BUS_Gyro[3], const
                boolean_T *rtu_IMU_BUS_Valid, const real32_T *rtu_GPS_BUS_Lat,
                const real32_T *rtu_GPS_BUS_Lon, const real32_T *rtu_GPS_BUS_Alt,
                const real32_T rtu_GPS_BUS_Velocity[3], const boolean_T
                *rtu_GPS_BUS_Valid, const boolean_T *rtu_GPS_BUS_IsNew, real32_T
                rty_EscCmdBus_MotorCmd[4], boolean_T *rty_EscCmdBus_Armed,
                boolean_T *rty_EscCmdBus_Valid, real32_T
                rty_StateEstBus_Position_NED[3], real32_T
                rty_StateEstBus_Velocity_NED[3], real32_T
                rty_StateEstBus_Attitude_quat[4], real32_T
                rty_StateEstBus_AngularRate_Bod[3], real32_T
                rty_StateEstBus_Accel_Body[3], real32_T
                rty_StateEstBus_GyroBias[3], real32_T rty_StateEstBus_AccelBias
                [3], real32_T rty_StateEstBus_Wind_NED[3], uint8_T
                *rty_StateEstBus_Status, B_FlightCore_c_T *localB,
                DW_FlightCore_f_T *localDW)
{
  localB->b = FlightCore_M->isMajorTimeStep();
  if (localB->b) {
    // ModelReference generated from: '<Root>/EKF'
    EKF(&rtu_IMU_BUS_Accel[0], &rtu_IMU_BUS_Gyro[0], rtu_IMU_BUS_Valid,
        rtu_GPS_BUS_Lat, rtu_GPS_BUS_Lon, rtu_GPS_BUS_Alt,
        &rtu_GPS_BUS_Velocity[0], rtu_GPS_BUS_Valid, rtu_GPS_BUS_IsNew,
        &rty_StateEstBus_Position_NED[0], &rty_StateEstBus_Velocity_NED[0],
        &rty_StateEstBus_Attitude_quat[0], &rty_StateEstBus_AngularRate_Bod[0],
        &rty_StateEstBus_Accel_Body[0], &rty_StateEstBus_GyroBias[0],
        &rty_StateEstBus_AccelBias[0], &rty_StateEstBus_Wind_NED[0],
        rty_StateEstBus_Status, &(localDW->EKF_InstanceData.rtb),
        &(localDW->EKF_InstanceData.rtdw));

    // BusCreator generated from: '<Root>/UAV_FlightControl'
    localB->BusConversion_InsertedFor_UAV_F.Position_NED[0] =
      rty_StateEstBus_Position_NED[0];
    localB->BusConversion_InsertedFor_UAV_F.Velocity_NED[0] =
      rty_StateEstBus_Velocity_NED[0];
    localB->BusConversion_InsertedFor_UAV_F.Position_NED[1] =
      rty_StateEstBus_Position_NED[1];
    localB->BusConversion_InsertedFor_UAV_F.Velocity_NED[1] =
      rty_StateEstBus_Velocity_NED[1];
    localB->BusConversion_InsertedFor_UAV_F.Position_NED[2] =
      rty_StateEstBus_Position_NED[2];
    localB->BusConversion_InsertedFor_UAV_F.Velocity_NED[2] =
      rty_StateEstBus_Velocity_NED[2];
    localB->BusConversion_InsertedFor_UAV_F.Attitude_quat[0] =
      rty_StateEstBus_Attitude_quat[0];
    localB->BusConversion_InsertedFor_UAV_F.Attitude_quat[1] =
      rty_StateEstBus_Attitude_quat[1];
    localB->BusConversion_InsertedFor_UAV_F.Attitude_quat[2] =
      rty_StateEstBus_Attitude_quat[2];
    localB->BusConversion_InsertedFor_UAV_F.Attitude_quat[3] =
      rty_StateEstBus_Attitude_quat[3];
    localB->BusConversion_InsertedFor_UAV_F.AngularRate_Body[0] =
      rty_StateEstBus_AngularRate_Bod[0];
    localB->BusConversion_InsertedFor_UAV_F.Accel_Body[0] =
      rty_StateEstBus_Accel_Body[0];
    localB->BusConversion_InsertedFor_UAV_F.GyroBias[0] =
      rty_StateEstBus_GyroBias[0];
    localB->BusConversion_InsertedFor_UAV_F.AccelBias[0] =
      rty_StateEstBus_AccelBias[0];
    localB->BusConversion_InsertedFor_UAV_F.Wind_NED[0] =
      rty_StateEstBus_Wind_NED[0];
    localB->BusConversion_InsertedFor_UAV_F.AngularRate_Body[1] =
      rty_StateEstBus_AngularRate_Bod[1];
    localB->BusConversion_InsertedFor_UAV_F.Accel_Body[1] =
      rty_StateEstBus_Accel_Body[1];
    localB->BusConversion_InsertedFor_UAV_F.GyroBias[1] =
      rty_StateEstBus_GyroBias[1];
    localB->BusConversion_InsertedFor_UAV_F.AccelBias[1] =
      rty_StateEstBus_AccelBias[1];
    localB->BusConversion_InsertedFor_UAV_F.Wind_NED[1] =
      rty_StateEstBus_Wind_NED[1];
    localB->BusConversion_InsertedFor_UAV_F.AngularRate_Body[2] =
      rty_StateEstBus_AngularRate_Bod[2];
    localB->BusConversion_InsertedFor_UAV_F.Accel_Body[2] =
      rty_StateEstBus_Accel_Body[2];
    localB->BusConversion_InsertedFor_UAV_F.GyroBias[2] =
      rty_StateEstBus_GyroBias[2];
    localB->BusConversion_InsertedFor_UAV_F.AccelBias[2] =
      rty_StateEstBus_AccelBias[2];
    localB->BusConversion_InsertedFor_UAV_F.Wind_NED[2] =
      rty_StateEstBus_Wind_NED[2];
    localB->BusConversion_InsertedFor_UAV_F.Status = *rty_StateEstBus_Status;
  }

  // Step: '<S1>/TestPositionCommandStep'
  localB->y = 0;

  // Saturate: '<S1>/LimitAltitudeCommandTo5m'
  localB->FlightCmdBus_Creator.Position_NED_SP[0] = 0.0F;
  localB->FlightCmdBus_Creator.Position_NED_SP[1] = 0.0F;

  // Step: '<S1>/TestPositionCommandStep'
  if (!((*(FlightCore_M->timingBridge->taskTime
           [FlightCore_M->Timing.mdlref_GlobalTID[0]])) < 5.0)) {
    localB->y = -5;
  }

  // Saturate: '<S1>/LimitAltitudeCommandTo5m'
  localB->FlightCmdBus_Creator.Position_NED_SP[2] = static_cast<real32_T>
    (localB->y);
  if (localB->b) {
    // DataTypeConversion: '<S1>/Data Type Conversion' incorporates:
    //   Step: '<S1>/TestArmRequestStep'

    localB->ArmRequest = !((FlightCore_M->getClockTick1() * 0.001) < 1.0);

    // RelationalOperator: '<S4>/Compare' incorporates:
    //   Constant: '<S4>/Constant'

    localB->Compare = (*rty_StateEstBus_Status == 1);
  }

  // BusCreator: '<S1>/FlightCmdBus_Creator' incorporates:
  //   Constant: '<S1>/CommandValid'
  //   Constant: '<S1>/ModePositionHold'
  //   Constant: '<S1>/VelocityZero'
  //   Constant: '<S1>/YawZero'

  localB->FlightCmdBus_Creator.Velocity_NED_SP[0] = 0.0F;
  localB->FlightCmdBus_Creator.Velocity_NED_SP[1] = 0.0F;
  localB->FlightCmdBus_Creator.Velocity_NED_SP[2] = 0.0F;
  localB->FlightCmdBus_Creator.Yaw_SP = 0.0F;
  localB->FlightCmdBus_Creator.Mode = 1U;
  localB->FlightCmdBus_Creator.ArmRequest = localB->ArmRequest;
  localB->FlightCmdBus_Creator.Valid = true;

  // Logic: '<S2>/Logical Operator'
  localB->Ready = ((*rtu_IMU_BUS_Valid) && localB->Compare);
  if (localB->b) {
    // Chart: '<S2>/Chart'
    resultZC0 = (static_cast<int32_T>(rt_ZCFcn(RISING_ZERO_CROSSING,
      &localDW->previousZC,
      (static_cast<real_T>(localB->FlightCmdBus_Creator.ArmRequest)))) != 0);
    if (localDW->is_active_c3_FlightCore == 0) {
      localDW->is_active_c3_FlightCore = 1U;
      localDW->is_c3_FlightCore = FlightCore_IN_DISARMED;
      localB->Armed = false;
      localB->ControlActive = false;
    } else {
      switch (localDW->is_c3_FlightCore) {
       case FlightCore_IN_ACTIVE:
        if (!localB->FlightCmdBus_Creator.ArmRequest) {
          localDW->is_c3_FlightCore = FlightCore_IN_DISARMED;
          localB->Armed = false;
          localB->ControlActive = false;
        } else if (localB->FlightCmdBus_Creator.ArmRequest && (!localB->Ready))
        {
          localDW->is_c3_FlightCore = FlightCore_IN_CONTROL_INHIBITED;
          localB->Armed = true;
          localB->ControlActive = false;
        } else {
          localB->Armed = true;
          localB->ControlActive = true;
        }
        break;

       case FlightCore_IN_CONTROL_INHIBITED:
        if (!localB->FlightCmdBus_Creator.ArmRequest) {
          localDW->is_c3_FlightCore = FlightCore_IN_DISARMED;
          localB->Armed = false;
          localB->ControlActive = false;
        } else {
          localB->Armed = true;
          localB->ControlActive = false;
        }
        break;

       default:
        // case IN_DISARMED:
        if (resultZC0 && localB->Ready) {
          localDW->is_c3_FlightCore = FlightCore_IN_ACTIVE;
          localB->Armed = true;
          localB->ControlActive = true;
        } else {
          localB->Armed = false;
          localB->ControlActive = false;
        }
        break;
      }
    }

    // End of Chart: '<S2>/Chart'
  }

  // ModelReference generated from: '<Root>/UAV_FlightControl'
  if (FlightCore_M->isMajorTimeStep() || FlightCore_M->isMajorTimeStep() &&
      FlightCore_M->isSampleHit(2) || FlightCore_M->isMajorTimeStep() &&
      FlightCore_M->isSampleHit(3)) {
    UAV_FlightControl(&(localDW->UAV_FlightControl_InstanceData.rtm),
                      &localB->BusConversion_InsertedFor_UAV_F,
                      &localB->FlightCmdBus_Creator.Position_NED_SP[0],
                      &localB->FlightCmdBus_Creator.Yaw_SP,
                      &localB->ControlActive, &localB->Armed,
                      &localB->EscCmdBus_g,
                      &(localDW->UAV_FlightControl_InstanceData.rtb),
                      &(localDW->UAV_FlightControl_InstanceData.rtdw));
  }

  if (localB->b) {
    // SignalConversion generated from: '<Root>/EscCmdBus'
    rty_EscCmdBus_MotorCmd[0] = localB->EscCmdBus_g.MotorCmd[0];
    rty_EscCmdBus_MotorCmd[1] = localB->EscCmdBus_g.MotorCmd[1];
    rty_EscCmdBus_MotorCmd[2] = localB->EscCmdBus_g.MotorCmd[2];
    rty_EscCmdBus_MotorCmd[3] = localB->EscCmdBus_g.MotorCmd[3];

    // SignalConversion generated from: '<Root>/EscCmdBus'
    *rty_EscCmdBus_Armed = localB->EscCmdBus_g.Armed;

    // SignalConversion generated from: '<Root>/EscCmdBus'
    *rty_EscCmdBus_Valid = localB->EscCmdBus_g.Valid;
  }
}

// Model initialize function
void FlightCore_initialize(const char_T **rt_errorStatus, RTWSolverInfo
  *rt_solverInfo, const rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T
  mdlref_TID1, int_T mdlref_TID2, int_T mdlref_TID3, RT_MODEL_FlightCore_T *
  const FlightCore_M, DW_FlightCore_f_T *localDW, ZCE_FlightCore_T *localZCE)
{
  // Registration code

  // setup the global timing engine
  FlightCore_M->Timing.mdlref_GlobalTID[0] = mdlref_TID0;
  FlightCore_M->Timing.mdlref_GlobalTID[1] = mdlref_TID1;
  FlightCore_M->Timing.mdlref_GlobalTID[2] = mdlref_TID2;
  FlightCore_M->Timing.mdlref_GlobalTID[3] = mdlref_TID3;
  FlightCore_M->timingBridge = (timingBridge);

  // initialize error status
  FlightCore_M->setErrorStatusPointer(rt_errorStatus);

  // initialize RTWSolverInfo
  FlightCore_M->solverInfo = (rt_solverInfo);

  // Set the Timing fields to the appropriate data in the RTWSolverInfo
  FlightCore_M->setSimTimeStepPointer(rtsiGetSimTimeStepPtr
    (FlightCore_M->solverInfo));
  FlightCore_M->Timing.stepSize0 = (rtsiGetStepSize(FlightCore_M->solverInfo));

  // Model Initialize function for ModelReference Block: '<Root>/EKF'
  EKF_initialize(FlightCore_M->getErrorStatusPointer(),
                 &(localDW->EKF_InstanceData.rtm));

  // Model Initialize function for ModelReference Block: '<Root>/UAV_FlightControl' 
  UAV_FlightControl_initialize(FlightCore_M->getErrorStatusPointer(),
    timingBridge, mdlref_TID1, mdlref_TID2, mdlref_TID3,
    &(localDW->UAV_FlightControl_InstanceData.rtm));
  localZCE->SFunction_edgeDetectionSignal_Z = UNINITIALIZED_ZCSIG;
}

time_T RT_MODEL_FlightCore_T::getClockTickH0() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[0]]) );
}

time_T RT_MODEL_FlightCore_T::getClockTick0() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[0]])) );
}

time_T RT_MODEL_FlightCore_T::getClockTickH1() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[1]]) );
}

time_T RT_MODEL_FlightCore_T::getClockTick1() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[1]])) );
}

time_T RT_MODEL_FlightCore_T::getClockTickH2() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[2]]) );
}

time_T RT_MODEL_FlightCore_T::getClockTick2() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[2]])) );
}

time_T RT_MODEL_FlightCore_T::getClockTickH3() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[3]]) );
}

time_T RT_MODEL_FlightCore_T::getClockTick3() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[3]])) );
}

const char_T* RT_MODEL_FlightCore_T::getErrorStatus() const
{
  return (*(errorStatus));
}

void RT_MODEL_FlightCore_T::setErrorStatus(const char_T* const aErrorStatus)
  const
{
  (*(errorStatus) = aErrorStatus);
}

const char_T** RT_MODEL_FlightCore_T::getErrorStatusPointer() const
{
  return errorStatus;
}

void RT_MODEL_FlightCore_T::setErrorStatusPointer(const char_T
  ** aErrorStatusPointer)
{
  (errorStatus = aErrorStatusPointer);
}

boolean_T RT_MODEL_FlightCore_T::isMajorTimeStep() const
{
  return ((getSimTimeStep()) == MAJOR_TIME_STEP);
}

boolean_T RT_MODEL_FlightCore_T::isMinorTimeStep() const
{
  return ((getSimTimeStep()) == MINOR_TIME_STEP);
}

boolean_T RT_MODEL_FlightCore_T::isSampleHit(int32_T sti) const
{
  return (timingBridge->taskCounter[Timing.mdlref_GlobalTID[sti]] == 0);
}

SimTimeStep RT_MODEL_FlightCore_T::getSimTimeStep() const
{
  return (*(Timing.simTimeStep));
}

SimTimeStep* RT_MODEL_FlightCore_T::getSimTimeStepPointer() const
{
  return Timing.simTimeStep;
}

void RT_MODEL_FlightCore_T::setSimTimeStepPointer(SimTimeStep*
  aSimTimeStepPointer)
{
  (Timing.simTimeStep = aSimTimeStepPointer);
}

time_T RT_MODEL_FlightCore_T::getT() const
{
  return (*(timingBridge->taskTime[0]));
}

//
// File trailer for generated code.
//
// [EOF]
//
