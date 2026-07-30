//
// File: UAV_FlightControl.cpp
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
#include "UAV_FlightControl.h"
#include "UAV_FlightControl_types.h"
#include "rtwtypes.h"
#include <xmmintrin.h>
#include <cmath>
#include "norm_94qjDDKI.h"

// System initialize for referenced model: 'UAV_FlightControl'
void UAV_FlightControl_Init(DW_UAV_FlightControl_f_T *localDW)
{
  // SystemInitialize for Enabled SubSystem: '<Root>/ControlLaw'
  // InitializeConditions for RateTransition: '<S2>/RT_RateSP_250To1kHz'
  localDW->RT_RateSP_250To1kHz_Buffer[0] = 0.0F;
  localDW->RT_RateSP_250To1kHz_Buffer[1] = 0.0F;
  localDW->RT_RateSP_250To1kHz_Buffer[2] = 0.0F;
  localDW->RT_RateSP_250To1kHz_ActiveBufId = 0;

  // InitializeConditions for RateTransition: '<S1>/RT_ThrustCmd_50To1kHz'
  localDW->RT_ThrustCmd_50To1kHz_Buffer0 = 0.0F;

  // InitializeConditions for RateTransition: '<S1>/RT_AccCmd_50To250Hz'
  localDW->RT_AccCmd_50To250Hz_Buffer[0] = 0.0F;
  localDW->RT_AccCmd_50To250Hz_Buffer[1] = 0.0F;
  localDW->RT_AccCmd_50To250Hz_Buffer[2] = 0.0F;
  localDW->RT_AccCmd_50To250Hz_ActiveBufId = 0;

  // End of SystemInitialize for SubSystem: '<Root>/ControlLaw'
}

// Disable for referenced model: 'UAV_FlightControl'
void UAV_FlightControl_Disable(B_UAV_FlightControl_c_T *localB,
  DW_UAV_FlightControl_f_T *localDW)
{
  // Disable for Enabled SubSystem: '<Root>/ControlLaw'
  if (localDW->ControlLaw_MODE) {
    // Disable for Outport: '<S1>/MotorCmd'
    localB->motor_cmd[0] = 0.0F;
    localB->motor_cmd[1] = 0.0F;
    localB->motor_cmd[2] = 0.0F;
    localB->motor_cmd[3] = 0.0F;
    localDW->ControlLaw_MODE = false;
  }

  // End of Disable for SubSystem: '<Root>/ControlLaw'
}

// Output and update for referenced model: 'UAV_FlightControl'
void UAV_FlightControl(RT_MODEL_UAV_FlightControl_T * const UAV_FlightControl_M,
  const StateEstBus *rtu_StateEstBus, const real32_T
  rtu_FlightCmdBus_Position_NED_S[3], const real32_T *rtu_FlightCmdBus_Yaw_SP,
  const boolean_T *rtu_ControlActive, const boolean_T *rtu_Armed, EscCmdBus
  *rty_EscCmdBus, B_UAV_FlightControl_c_T *localB, DW_UAV_FlightControl_f_T
  *localDW)
{
  __m128 tmp_3;
  int32_T skew_error_Body_tmp;
  real32_T q_idx_0;
  real32_T rtb_DeadZone_a_idx_1;
  real32_T rtb_DeadZone_g_idx_0;
  real32_T rtb_DeadZone_g_idx_1;
  real32_T rtb_DerivativeGain_i_idx_0;
  real32_T rtb_DerivativeGain_i_idx_1;
  real32_T tmp_4;
  int8_T tmp_1;
  int8_T tmp_2;
  boolean_T tmp;
  boolean_T tmp_0;

  // Outputs for Enabled SubSystem: '<Root>/ControlLaw' incorporates:
  //   EnablePort: '<S1>/Enable'

  if (*rtu_ControlActive) {
    if (!localDW->ControlLaw_MODE) {
      // InitializeConditions for RateTransition: '<S2>/RT_RateSP_250To1kHz'
      localDW->RT_RateSP_250To1kHz_Buffer[0] = 0.0F;
      localDW->RT_RateSP_250To1kHz_Buffer[1] = 0.0F;
      localDW->RT_RateSP_250To1kHz_Buffer[2] = 0.0F;
      localDW->RT_RateSP_250To1kHz_ActiveBufId = 0;

      // InitializeConditions for DiscreteIntegrator: '<S103>/Integrator'
      localDW->Integrator_DSTATE = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S96>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_states = 0.0F;

      // InitializeConditions for DiscreteIntegrator: '<S159>/Integrator'
      localDW->Integrator_DSTATE_g = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S152>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_states_h = 0.0F;

      // InitializeConditions for DiscreteIntegrator: '<S215>/Integrator'
      localDW->Integrator_DSTATE_a = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S208>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_states_l = 0.0F;

      // InitializeConditions for RateTransition: '<S1>/RT_ThrustCmd_50To1kHz'
      localDW->RT_ThrustCmd_50To1kHz_Buffer0 = 0.0F;

      // InitializeConditions for RateTransition: '<S1>/RT_AccCmd_50To250Hz'
      localDW->RT_AccCmd_50To250Hz_Buffer[0] = 0.0F;
      localDW->RT_AccCmd_50To250Hz_Buffer[1] = 0.0F;
      localDW->RT_AccCmd_50To250Hz_Buffer[2] = 0.0F;
      localDW->RT_AccCmd_50To250Hz_ActiveBufId = 0;

      // InitializeConditions for DiscreteIntegrator: '<S377>/Integrator'
      localDW->Integrator_DSTATE_k[0] = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S370>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_state_hl[0] = 0.0F;

      // InitializeConditions for DiscreteIntegrator: '<S492>/Integrator'
      localDW->Integrator_DSTATE_m[0] = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S485>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_states_m[0] = 0.0F;

      // InitializeConditions for DiscreteIntegrator: '<S377>/Integrator'
      localDW->Integrator_DSTATE_k[1] = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S370>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_state_hl[1] = 0.0F;

      // InitializeConditions for DiscreteIntegrator: '<S492>/Integrator'
      localDW->Integrator_DSTATE_m[1] = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S485>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_states_m[1] = 0.0F;

      // InitializeConditions for DiscreteIntegrator: '<S433>/Integrator'
      localDW->Integrator_DSTATE_o = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S426>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_states_a = 0.0F;

      // InitializeConditions for DiscreteTransferFcn: '<S541>/Filter Differentiator TF' 
      localDW->FilterDifferentiatorTF_states_e = 0.0F;

      // InitializeConditions for DiscreteIntegrator: '<S548>/Integrator'
      localDW->Integrator_DSTATE_p = 0.0F;
      localDW->ControlLaw_MODE = true;
    }

    // RateTransition: '<S2>/RT_RateSP_250To1kHz'
    localB->i = localDW->RT_RateSP_250To1kHz_ActiveBufId * 3;
    localB->RT_RateSP_250To1kHz[0] = localDW->RT_RateSP_250To1kHz_Buffer
      [localB->i];
    localB->RT_RateSP_250To1kHz[1] = localDW->RT_RateSP_250To1kHz_Buffer
      [localB->i + 1];
    localB->RT_RateSP_250To1kHz[2] = localDW->RT_RateSP_250To1kHz_Buffer
      [localB->i + 2];

    // Sum: '<S2>/SumRateP'
    localB->IntegralGain = localB->RT_RateSP_250To1kHz[0] -
      rtu_StateEstBus->AngularRate_Body[0];

    // Math: '<S96>/Reciprocal' incorporates:
    //   Constant: '<S105>/N Copy'
    //   Constant: '<S96>/Filter Den Constant'
    //   Math: '<S152>/Reciprocal'
    //   Math: '<S208>/Reciprocal'
    //   SampleTimeMath: '<S98>/Tsamp'
    //   Sum: '<S96>/SumDen'
    //
    //  About '<S96>/Reciprocal':
    //   Operator: reciprocal
    //
    //  About '<S152>/Reciprocal':
    //   Operator: reciprocal
    //
    //  About '<S208>/Reciprocal':
    //   Operator: reciprocal
    //
    //  About '<S98>/Tsamp':
    //   y = u * K where K = ( w * Ts )
    //
    localB->DeadZone_o = 1.0F / (rateFilterN * 0.001F + 1.0F);

    // DiscreteTransferFcn: '<S96>/Filter Differentiator TF' incorporates:
    //   Gain: '<S94>/Derivative Gain'
    //   Math: '<S96>/Reciprocal'
    //   UnaryMinus: '<S96>/Unary Minus'
    //
    //  About '<S96>/Reciprocal':
    //   Operator: reciprocal

    localDW->FilterDifferentiatorTF_tmp = rateD * localB->IntegralGain -
      -localB->DeadZone_o * localDW->FilterDifferentiatorTF_states;

    // Sum: '<S112>/Sum' incorporates:
    //   DiscreteIntegrator: '<S103>/Integrator'
    //   DiscreteTransferFcn: '<S96>/Filter Differentiator TF'
    //   Gain: '<S106>/Filter Coefficient'
    //   Gain: '<S108>/Proportional Gain'
    //   Math: '<S96>/Reciprocal'
    //   Product: '<S96>/DenCoefOut'
    //
    //  About '<S96>/Reciprocal':
    //   Operator: reciprocal

    localB->DeadZone = (localDW->FilterDifferentiatorTF_tmp -
                        localDW->FilterDifferentiatorTF_states) *
      localB->DeadZone_o * rateFilterN + (rateP * localB->IntegralGain +
      localDW->Integrator_DSTATE);

    // Sum: '<S2>/SumRateQ'
    localB->IntegralGain_f = localB->RT_RateSP_250To1kHz[1] -
      rtu_StateEstBus->AngularRate_Body[1];

    // DiscreteTransferFcn: '<S152>/Filter Differentiator TF' incorporates:
    //   Gain: '<S150>/Derivative Gain'
    //   UnaryMinus: '<S152>/Unary Minus'

    localDW->FilterDifferentiatorTF_tmp_b = rateD * localB->IntegralGain_f -
      -localB->DeadZone_o * localDW->FilterDifferentiatorTF_states_h;

    // Sum: '<S168>/Sum' incorporates:
    //   DiscreteIntegrator: '<S159>/Integrator'
    //   DiscreteTransferFcn: '<S152>/Filter Differentiator TF'
    //   Gain: '<S162>/Filter Coefficient'
    //   Gain: '<S164>/Proportional Gain'
    //   Product: '<S152>/DenCoefOut'

    localB->DeadZone_i = (localDW->FilterDifferentiatorTF_tmp_b -
                          localDW->FilterDifferentiatorTF_states_h) *
      localB->DeadZone_o * rateFilterN + (rateP * localB->IntegralGain_f +
      localDW->Integrator_DSTATE_g);

    // Sum: '<S2>/SumRateR'
    localB->IntegralGain_h = localB->RT_RateSP_250To1kHz[2] -
      rtu_StateEstBus->AngularRate_Body[2];

    // DiscreteTransferFcn: '<S208>/Filter Differentiator TF' incorporates:
    //   Gain: '<S206>/Derivative Gain'
    //   UnaryMinus: '<S208>/Unary Minus'

    localDW->FilterDifferentiatorTF_tmp_g = rateD * localB->IntegralGain_h -
      -localB->DeadZone_o * localDW->FilterDifferentiatorTF_states_l;

    // Sum: '<S224>/Sum' incorporates:
    //   DiscreteIntegrator: '<S215>/Integrator'
    //   DiscreteTransferFcn: '<S208>/Filter Differentiator TF'
    //   Gain: '<S218>/Filter Coefficient'
    //   Gain: '<S220>/Proportional Gain'
    //   Product: '<S208>/DenCoefOut'

    localB->DeadZone_o = (localDW->FilterDifferentiatorTF_tmp_g -
                          localDW->FilterDifferentiatorTF_states_l) *
      localB->DeadZone_o * rateFilterN + (rateP * localB->IntegralGain_h +
      localDW->Integrator_DSTATE_a);

    // RateTransition: '<S1>/RT_ThrustCmd_50To1kHz'
    localB->RT_ThrustCmd_50To1kHz = localDW->RT_ThrustCmd_50To1kHz_Buffer0;

    // Saturate: '<S110>/Saturation'
    if (localB->DeadZone > maxTorque) {
      localB->q_norm = maxTorque;
    } else if (localB->DeadZone < -2.0F) {
      localB->q_norm = -2.0F;
    } else {
      localB->q_norm = localB->DeadZone;
    }

    // End of Saturate: '<S110>/Saturation'

    // Saturate: '<S2>/Sat_TorqueX'
    if (localB->q_norm > maxTorque) {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[0] = maxTorque;
    } else if (localB->q_norm < -maxTorque) {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[0] = -maxTorque;
    } else {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[0] = localB->q_norm;
    }

    // End of Saturate: '<S2>/Sat_TorqueX'

    // Saturate: '<S166>/Saturation'
    if (localB->DeadZone_i > maxTorque) {
      localB->q_norm = maxTorque;
    } else if (localB->DeadZone_i < -2.0F) {
      localB->q_norm = -2.0F;
    } else {
      localB->q_norm = localB->DeadZone_i;
    }

    // End of Saturate: '<S166>/Saturation'

    // Saturate: '<S2>/Sat_TorqueY'
    if (localB->q_norm > maxTorque) {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[1] = maxTorque;
    } else if (localB->q_norm < -maxTorque) {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[1] = -maxTorque;
    } else {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[1] = localB->q_norm;
    }

    // End of Saturate: '<S2>/Sat_TorqueY'

    // Saturate: '<S222>/Saturation'
    if (localB->DeadZone_o > maxTorque) {
      localB->q_norm = maxTorque;
    } else if (localB->DeadZone_o < -2.0F) {
      localB->q_norm = -2.0F;
    } else {
      localB->q_norm = localB->DeadZone_o;
    }

    // End of Saturate: '<S222>/Saturation'

    // Saturate: '<S2>/Sat_TorqueZ'
    if (localB->q_norm > maxTorque) {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[2] = maxTorque;
    } else if (localB->q_norm < -maxTorque) {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[2] = -maxTorque;
    } else {
      // SignalConversion generated from: '<S3>/ SFunction ' incorporates:
      //   MATLAB Function: '<S1>/Mixer'

      localB->RT_RateSP_250To1kHz[2] = localB->q_norm;
    }

    // End of Saturate: '<S2>/Sat_TorqueZ'

    // MATLAB Function: '<S1>/Mixer' incorporates:
    //   Constant: '<S1>/MixMatrix'
    //   Constant: '<S1>/MotorArmLength_m'
    //   Constant: '<S1>/MotorMax'
    //   Constant: '<S1>/MotorMaxReactionTorque_Nm'
    //   Constant: '<S1>/MotorMaxThrust_N'
    //   Constant: '<S1>/MotorMin'

    localB->rtb_RT_ThrustCmd_50To1kHz_m[0] = localB->RT_ThrustCmd_50To1kHz /
      (4.0F * motorMaxThrust_N);
    localB->q_norm = motorArmLength_m / 1.41421354F * 4.0F * motorMaxThrust_N;
    localB->rtb_RT_ThrustCmd_50To1kHz_m[1] = localB->RT_RateSP_250To1kHz[0] /
      localB->q_norm;
    localB->rtb_RT_ThrustCmd_50To1kHz_m[2] = localB->RT_RateSP_250To1kHz[1] /
      localB->q_norm;
    localB->rtb_RT_ThrustCmd_50To1kHz_m[3] = localB->RT_RateSP_250To1kHz[2] /
      (4.0F * motorMaxReactionTorque_Nm);
    localB->q_norm = 0.0F;
    localB->RT_ThrustCmd_50To1kHz = 0.0F;
    localB->scale = 0.0F;
    localB->absxk = 0.0F;
    for (localB->i = 0; localB->i < 4; localB->i++) {
      _mm_storeu_ps(&localB->fv[0], _mm_add_ps(_mm_mul_ps(_mm_loadu_ps
        (&mixMatrix[localB->i << 2]), _mm_set1_ps
        (localB->rtb_RT_ThrustCmd_50To1kHz_m[localB->i])), _mm_set_ps
        (localB->absxk, localB->scale, localB->RT_ThrustCmd_50To1kHz,
         localB->q_norm)));
      localB->q_norm = localB->fv[0];
      localB->RT_ThrustCmd_50To1kHz = localB->fv[1];
      localB->scale = localB->fv[2];
      localB->absxk = localB->fv[3];
    }

    localB->motor_cmd[3] = localB->absxk;
    localB->motor_cmd[2] = localB->scale;
    localB->motor_cmd[1] = localB->RT_ThrustCmd_50To1kHz;
    localB->motor_cmd[0] = localB->q_norm;
    localB->motor_cmd[0] = std::fmax(motorMin, std::fmin(motorMax,
      localB->motor_cmd[0]));
    localB->motor_cmd[1] = std::fmax(motorMin, std::fmin(motorMax,
      localB->motor_cmd[1]));
    localB->motor_cmd[2] = std::fmax(motorMin, std::fmin(motorMax,
      localB->motor_cmd[2]));
    localB->motor_cmd[3] = std::fmax(motorMin, std::fmin(motorMax,
      localB->motor_cmd[3]));

    // DeadZone: '<S205>/DeadZone'
    if (localB->DeadZone_o > maxTorque) {
      localB->DeadZone_o -= maxTorque;
    } else if (localB->DeadZone_o >= -2.0F) {
      localB->DeadZone_o = 0.0F;
    } else {
      localB->DeadZone_o -= -2.0F;
    }

    // End of DeadZone: '<S205>/DeadZone'

    // Gain: '<S212>/Integral Gain'
    localB->IntegralGain_h *= rateI;

    // DeadZone: '<S149>/DeadZone'
    if (localB->DeadZone_i > maxTorque) {
      localB->DeadZone_i -= maxTorque;
    } else if (localB->DeadZone_i >= -2.0F) {
      localB->DeadZone_i = 0.0F;
    } else {
      localB->DeadZone_i -= -2.0F;
    }

    // End of DeadZone: '<S149>/DeadZone'

    // Gain: '<S156>/Integral Gain'
    localB->IntegralGain_f *= rateI;

    // DeadZone: '<S93>/DeadZone'
    if (localB->DeadZone > maxTorque) {
      localB->DeadZone -= maxTorque;
    } else if (localB->DeadZone >= -2.0F) {
      localB->DeadZone = 0.0F;
    } else {
      localB->DeadZone -= -2.0F;
    }

    // End of DeadZone: '<S93>/DeadZone'

    // Gain: '<S100>/Integral Gain'
    localB->IntegralGain *= rateI;

    // RateTransition: '<S1>/RT_AccCmd_50To250Hz' incorporates:
    //   RateTransition: '<S2>/RT_RateSP_250To1kHz'

    tmp = UAV_FlightControl_M->isSampleHit(1);
    if (tmp) {
      localB->i = localDW->RT_AccCmd_50To250Hz_ActiveBufId * 3;
      localB->RT_RateSP_250To1kHz[0] = localDW->
        RT_AccCmd_50To250Hz_Buffer[localB->i];
      localB->RT_RateSP_250To1kHz[1] = localDW->
        RT_AccCmd_50To250Hz_Buffer[localB->i + 1];
      localB->RT_RateSP_250To1kHz[2] = localDW->
        RT_AccCmd_50To250Hz_Buffer[localB->i + 2];

      // RateTransition: '<S1>/RT_YawCmd_1kTo250Hz'
      localDW->RT_YawCmd_1kTo250Hz_Buffer = *rtu_FlightCmdBus_Yaw_SP;

      // MATLAB Function: '<S2>/AccelToAttitude'
      localB->scale = 1.29246971E-26F;

      // RateTransition: '<S1>/RT_AttitudeEst_1kTo250Hz'
      localDW->RT_AttitudeEst_1kTo250Hz_Buffer[0] =
        rtu_StateEstBus->Attitude_quat[0];

      // MATLAB Function: '<S2>/AccelToAttitude' incorporates:
      //   RateTransition: '<S1>/RT_AttitudeEst_1kTo250Hz'

      localB->absxk = std::abs(localDW->RT_AttitudeEst_1kTo250Hz_Buffer[0]);
      if (localB->absxk > 1.29246971E-26F) {
        localB->q_norm = 1.0F;
        localB->scale = localB->absxk;
      } else {
        localB->t = localB->absxk / 1.29246971E-26F;
        localB->q_norm = localB->t * localB->t;
      }

      // RateTransition: '<S1>/RT_AttitudeEst_1kTo250Hz'
      localDW->RT_AttitudeEst_1kTo250Hz_Buffer[1] =
        rtu_StateEstBus->Attitude_quat[1];

      // MATLAB Function: '<S2>/AccelToAttitude' incorporates:
      //   RateTransition: '<S1>/RT_AttitudeEst_1kTo250Hz'

      localB->absxk = std::abs(localDW->RT_AttitudeEst_1kTo250Hz_Buffer[1]);
      if (localB->absxk > localB->scale) {
        localB->t = localB->scale / localB->absxk;
        localB->q_norm = localB->q_norm * localB->t * localB->t + 1.0F;
        localB->scale = localB->absxk;
      } else {
        localB->t = localB->absxk / localB->scale;
        localB->q_norm += localB->t * localB->t;
      }

      // RateTransition: '<S1>/RT_AttitudeEst_1kTo250Hz'
      localDW->RT_AttitudeEst_1kTo250Hz_Buffer[2] =
        rtu_StateEstBus->Attitude_quat[2];

      // MATLAB Function: '<S2>/AccelToAttitude' incorporates:
      //   RateTransition: '<S1>/RT_AttitudeEst_1kTo250Hz'

      localB->absxk = std::abs(localDW->RT_AttitudeEst_1kTo250Hz_Buffer[2]);
      if (localB->absxk > localB->scale) {
        localB->t = localB->scale / localB->absxk;
        localB->q_norm = localB->q_norm * localB->t * localB->t + 1.0F;
        localB->scale = localB->absxk;
      } else {
        localB->t = localB->absxk / localB->scale;
        localB->q_norm += localB->t * localB->t;
      }

      // RateTransition: '<S1>/RT_AttitudeEst_1kTo250Hz'
      localDW->RT_AttitudeEst_1kTo250Hz_Buffer[3] =
        rtu_StateEstBus->Attitude_quat[3];

      // MATLAB Function: '<S2>/AccelToAttitude' incorporates:
      //   Constant: '<S2>/Constant'
      //   RateTransition: '<S1>/RT_AttitudeEst_1kTo250Hz'
      //   RateTransition: '<S1>/RT_YawCmd_1kTo250Hz'

      localB->absxk = std::abs(localDW->RT_AttitudeEst_1kTo250Hz_Buffer[3]);
      if (localB->absxk > localB->scale) {
        localB->t = localB->scale / localB->absxk;
        localB->q_norm = localB->q_norm * localB->t * localB->t + 1.0F;
        localB->scale = localB->absxk;
      } else {
        localB->t = localB->absxk / localB->scale;
        localB->q_norm += localB->t * localB->t;
      }

      localB->q_norm = localB->scale * std::sqrt(localB->q_norm);
      if (localB->q_norm < 1.0E-6F) {
        q_idx_0 = 1.0F;
        localB->q_idx_1 = 0.0F;
        localB->q_idx_2 = 0.0F;
        localB->RT_ThrustCmd_50To1kHz = 0.0F;
      } else {
        tmp_3 = _mm_div_ps(_mm_loadu_ps
                           (&localDW->RT_AttitudeEst_1kTo250Hz_Buffer[0]),
                           _mm_set1_ps(localB->q_norm));
        _mm_storeu_ps(&localB->fv[0], tmp_3);
        q_idx_0 = localB->fv[0];
        localB->q_idx_1 = localB->fv[1];
        localB->q_idx_2 = localB->fv[2];
        localB->RT_ThrustCmd_50To1kHz = localB->fv[3];
      }

      localB->RT_RateSP_250To1kHz[0] = 0.0F - localB->RT_RateSP_250To1kHz[0];
      localB->RT_RateSP_250To1kHz[1] = 0.0F - localB->RT_RateSP_250To1kHz[1];
      localB->RT_RateSP_250To1kHz[2] = 9.80665F - localB->RT_RateSP_250To1kHz[2];
      localB->q_norm = norm_94qjDDKI(localB->RT_RateSP_250To1kHz);
      if (localB->q_norm < 1.0E-6F) {
        localB->RT_RateSP_250To1kHz[0] = 0.0F;
        localB->RT_RateSP_250To1kHz[1] = 0.0F;
        localB->RT_RateSP_250To1kHz[2] = 1.0F;
      } else {
        localB->RT_RateSP_250To1kHz[0] /= localB->q_norm;
        localB->RT_RateSP_250To1kHz[1] /= localB->q_norm;
        localB->RT_RateSP_250To1kHz[2] /= localB->q_norm;
      }

      localB->q_norm = std::sin(localDW->RT_YawCmd_1kTo250Hz_Buffer);
      localB->scale = std::cos(localDW->RT_YawCmd_1kTo250Hz_Buffer);
      localB->absxk = localB->RT_RateSP_250To1kHz[1] * 0.0F;
      localB->b2_des_NED_c[0] = localB->absxk - localB->q_norm *
        localB->RT_RateSP_250To1kHz[2];
      localB->t = localB->scale * localB->RT_RateSP_250To1kHz[2];
      localB->DeadZone_gb = localB->RT_RateSP_250To1kHz[0] * 0.0F;
      localB->b2_des_NED_c[1] = localB->t - localB->DeadZone_gb;
      localB->b2_des_NED_c[2] = localB->RT_RateSP_250To1kHz[0] * localB->q_norm
        - localB->scale * localB->RT_RateSP_250To1kHz[1];
      localB->b2_norm = norm_94qjDDKI(localB->b2_des_NED_c);
      if (localB->b2_norm < 1.0E-6F) {
        localB->b2_des_NED_c[0] = localB->absxk - localB->t;
        localB->b2_des_NED_c[1] = -localB->q_norm * localB->RT_RateSP_250To1kHz
          [2] - localB->DeadZone_gb;
        localB->b2_des_NED_c[2] = localB->RT_RateSP_250To1kHz[0] * localB->scale
          - -localB->q_norm * localB->RT_RateSP_250To1kHz[1];
        localB->b2_norm = norm_94qjDDKI(localB->b2_des_NED_c);
      }

      localB->q_norm = std::fmax(localB->b2_norm, 1.0E-6F);
      localB->scale = localB->RT_ThrustCmd_50To1kHz *
        localB->RT_ThrustCmd_50To1kHz;
      localB->absxk = localB->q_idx_2 * localB->q_idx_2;
      localB->skew_error_Body[0] = 1.0F - (localB->absxk + localB->scale) * 2.0F;
      localB->t = localB->q_idx_1 * localB->q_idx_2;
      localB->DeadZone_gb = q_idx_0 * localB->RT_ThrustCmd_50To1kHz;
      localB->skew_error_Body[1] = (localB->t - localB->DeadZone_gb) * 2.0F;
      localB->b2_norm = localB->q_idx_1 * localB->RT_ThrustCmd_50To1kHz;
      tmp_4 = q_idx_0 * localB->q_idx_2;
      localB->skew_error_Body[2] = (localB->b2_norm + tmp_4) * 2.0F;
      localB->skew_error_Body[3] = (localB->t + localB->DeadZone_gb) * 2.0F;
      localB->t = localB->q_idx_1 * localB->q_idx_1;
      localB->skew_error_Body[4] = 1.0F - (localB->t + localB->scale) * 2.0F;
      localB->scale = localB->q_idx_2 * localB->RT_ThrustCmd_50To1kHz;
      localB->DeadZone_gb = q_idx_0 * localB->q_idx_1;
      localB->skew_error_Body[5] = (localB->scale - localB->DeadZone_gb) * 2.0F;
      localB->skew_error_Body[6] = (localB->b2_norm - tmp_4) * 2.0F;
      localB->skew_error_Body[7] = (localB->scale + localB->DeadZone_gb) * 2.0F;
      localB->skew_error_Body[8] = 1.0F - (localB->t + localB->absxk) * 2.0F;
      q_idx_0 = localB->b2_des_NED_c[0] / localB->q_norm;
      localB->b2_des_NED_c[0] = q_idx_0;
      localB->b2_des_NED[3] = q_idx_0;
      localB->b2_des_NED[6] = localB->RT_RateSP_250To1kHz[0];
      q_idx_0 = localB->b2_des_NED_c[1] / localB->q_norm;
      localB->b2_des_NED_c[1] = q_idx_0;
      localB->b2_des_NED[4] = q_idx_0;
      localB->b2_des_NED[7] = localB->RT_RateSP_250To1kHz[1];
      q_idx_0 = localB->b2_des_NED_c[2] / localB->q_norm;
      localB->b2_des_NED[5] = q_idx_0;
      localB->b2_des_NED[8] = localB->RT_RateSP_250To1kHz[2];
      localB->b2_des_NED[0] = localB->b2_des_NED_c[1] *
        localB->RT_RateSP_250To1kHz[2] - localB->RT_RateSP_250To1kHz[1] *
        q_idx_0;
      localB->b2_des_NED[1] = localB->RT_RateSP_250To1kHz[0] * q_idx_0 -
        localB->b2_des_NED_c[0] * localB->RT_RateSP_250To1kHz[2];
      localB->b2_des_NED[2] = localB->b2_des_NED_c[0] *
        localB->RT_RateSP_250To1kHz[1] - localB->RT_RateSP_250To1kHz[0] *
        localB->b2_des_NED_c[1];
      for (localB->i = 0; localB->i < 3; localB->i++) {
        localB->q_norm = 0.0F;
        q_idx_0 = 0.0F;
        localB->q_idx_1 = 0.0F;
        for (skew_error_Body_tmp = 0; skew_error_Body_tmp < 3;
             skew_error_Body_tmp++) {
          localB->q_idx_2 = localB->b2_des_NED[3 * localB->i +
            skew_error_Body_tmp];
          localB->q_norm += localB->skew_error_Body[3 * skew_error_Body_tmp] *
            localB->q_idx_2;
          q_idx_0 += localB->skew_error_Body[3 * skew_error_Body_tmp + 1] *
            localB->q_idx_2;
          localB->q_idx_1 += localB->skew_error_Body[3 * skew_error_Body_tmp + 2]
            * localB->q_idx_2;
        }

        localB->R_error_Body[3 * localB->i + 2] = localB->q_idx_1;
        localB->R_error_Body[3 * localB->i + 1] = q_idx_0;
        localB->R_error_Body[3 * localB->i] = localB->q_norm;
      }

      for (localB->i = 0; localB->i < 3; localB->i++) {
        localB->skew_error_Body[3 * localB->i] = (localB->R_error_Body[3 *
          localB->i] - localB->R_error_Body[localB->i]) * 0.5F;
        skew_error_Body_tmp = 3 * localB->i + 1;
        localB->skew_error_Body[skew_error_Body_tmp] = (localB->
          R_error_Body[skew_error_Body_tmp] - localB->R_error_Body[localB->i + 3])
          * 0.5F;
        skew_error_Body_tmp = 3 * localB->i + 2;
        localB->skew_error_Body[skew_error_Body_tmp] = (localB->
          R_error_Body[skew_error_Body_tmp] - localB->R_error_Body[localB->i + 6])
          * 0.5F;
      }

      localB->RT_RateSP_250To1kHz[0] = localB->skew_error_Body[5];
      localB->RT_RateSP_250To1kHz[1] = localB->skew_error_Body[6];
      localB->RT_RateSP_250To1kHz[2] = localB->skew_error_Body[1];

      // Gain: '<S52>/Proportional Gain'
      q_idx_0 = attP * localB->RT_RateSP_250To1kHz[1];

      // Saturate: '<S2>/Sat_RateQ'
      if (q_idx_0 > maxRate) {
        q_idx_0 = maxRate;
      } else if (q_idx_0 < -maxRate) {
        q_idx_0 = -maxRate;
      }

      // End of Saturate: '<S2>/Sat_RateQ'

      // Gain: '<S272>/Proportional Gain'
      localB->q_idx_1 = attP * localB->RT_RateSP_250To1kHz[0];

      // Saturate: '<S2>/Sat_RateP'
      if (localB->q_idx_1 > maxRate) {
        localB->q_idx_1 = maxRate;
      } else if (localB->q_idx_1 < -maxRate) {
        localB->q_idx_1 = -maxRate;
      }

      // End of Saturate: '<S2>/Sat_RateP'

      // Gain: '<S324>/Proportional Gain'
      localB->q_idx_2 = attP * localB->RT_RateSP_250To1kHz[2];

      // Saturate: '<S2>/Sat_RateR'
      if (localB->q_idx_2 > maxRate) {
        localB->q_idx_2 = maxRate;
      } else if (localB->q_idx_2 < -maxRate) {
        localB->q_idx_2 = -maxRate;
      }

      // End of Saturate: '<S2>/Sat_RateR'
    }

    // End of RateTransition: '<S1>/RT_AccCmd_50To250Hz'

    // Update for RateTransition: '<S1>/RT_ThrustCmd_50To1kHz'
    tmp_0 = UAV_FlightControl_M->isSampleHit(2);
    if (tmp_0) {
      // RateTransition: '<S1>/RT_PositionCmd_1kTo50Hz'
      localDW->RT_PositionCmd_1kTo50Hz_Buffer[0] =
        rtu_FlightCmdBus_Position_NED_S[0];

      // RateTransition: '<S1>/RT_PositionEst_1kTo50Hz'
      localDW->RT_PositionEst_1kTo50Hz_Buffer[0] = rtu_StateEstBus->
        Position_NED[0];

      // RateTransition: '<S1>/RT_PositionCmd_1kTo50Hz'
      localDW->RT_PositionCmd_1kTo50Hz_Buffer[1] =
        rtu_FlightCmdBus_Position_NED_S[1];

      // RateTransition: '<S1>/RT_PositionEst_1kTo50Hz'
      localDW->RT_PositionEst_1kTo50Hz_Buffer[1] = rtu_StateEstBus->
        Position_NED[1];

      // RateTransition: '<S1>/RT_PositionCmd_1kTo50Hz'
      localDW->RT_PositionCmd_1kTo50Hz_Buffer[2] =
        rtu_FlightCmdBus_Position_NED_S[2];

      // RateTransition: '<S1>/RT_PositionEst_1kTo50Hz'
      localDW->RT_PositionEst_1kTo50Hz_Buffer[2] = rtu_StateEstBus->
        Position_NED[2];

      // Math: '<S370>/Reciprocal' incorporates:
      //   Constant: '<S370>/Filter Den Constant'
      //   Constant: '<S379>/N Copy'
      //   Math: '<S426>/Reciprocal'
      //   SampleTimeMath: '<S372>/Tsamp'
      //   Sum: '<S370>/SumDen'
      //
      //  About '<S370>/Reciprocal':
      //   Operator: reciprocal
      //
      //  About '<S426>/Reciprocal':
      //   Operator: reciprocal
      //
      //  About '<S372>/Tsamp':
      //   y = u * K where K = ( w * Ts )
      //
      localB->Sat_acc_vert = 1.0F / (posFilterN * 0.02F + 1.0F);

      // Sum: '<S4>/Sum' incorporates:
      //   RateTransition: '<S1>/RT_PositionCmd_1kTo50Hz'
      //   RateTransition: '<S1>/RT_PositionEst_1kTo50Hz'

      localB->thrust_sp = localDW->RT_PositionCmd_1kTo50Hz_Buffer[0] -
        localDW->RT_PositionEst_1kTo50Hz_Buffer[0];
      localB->TmpSignalConversionAtFilterDiff = localB->thrust_sp;

      // DiscreteTransferFcn: '<S370>/Filter Differentiator TF' incorporates:
      //   Gain: '<S368>/Derivative Gain'
      //   Math: '<S370>/Reciprocal'
      //   UnaryMinus: '<S370>/Unary Minus'
      //
      //  About '<S370>/Reciprocal':
      //   Operator: reciprocal

      rtb_DerivativeGain_i_idx_1 = posD_xy * localB->thrust_sp -
        -localB->Sat_acc_vert * localDW->FilterDifferentiatorTF_state_hl[0];
      localDW->FilterDifferentiatorTF_tmp_j[0] = rtb_DerivativeGain_i_idx_1;

      // Sum: '<S386>/Sum' incorporates:
      //   DiscreteIntegrator: '<S377>/Integrator'
      //   DiscreteTransferFcn: '<S370>/Filter Differentiator TF'
      //   Gain: '<S380>/Filter Coefficient'
      //   Gain: '<S382>/Proportional Gain'
      //   Math: '<S370>/Reciprocal'
      //   Product: '<S370>/DenCoefOut'
      //
      //  About '<S370>/Reciprocal':
      //   Operator: reciprocal

      localB->scale = (rtb_DerivativeGain_i_idx_1 -
                       localDW->FilterDifferentiatorTF_state_hl[0]) *
        localB->Sat_acc_vert * posFilterN + (posP_xy * localB->thrust_sp +
        localDW->Integrator_DSTATE_k[0]);

      // Sum: '<S4>/Sum' incorporates:
      //   RateTransition: '<S1>/RT_PositionCmd_1kTo50Hz'
      //   RateTransition: '<S1>/RT_PositionEst_1kTo50Hz'

      localB->thrust_sp = localDW->RT_PositionCmd_1kTo50Hz_Buffer[1] -
        localDW->RT_PositionEst_1kTo50Hz_Buffer[1];

      // DiscreteTransferFcn: '<S370>/Filter Differentiator TF' incorporates:
      //   Gain: '<S368>/Derivative Gain'
      //   Math: '<S370>/Reciprocal'
      //   UnaryMinus: '<S370>/Unary Minus'
      //
      //  About '<S370>/Reciprocal':
      //   Operator: reciprocal

      rtb_DerivativeGain_i_idx_1 = posD_xy * localB->thrust_sp -
        -localB->Sat_acc_vert * localDW->FilterDifferentiatorTF_state_hl[1];
      localDW->FilterDifferentiatorTF_tmp_j[1] = rtb_DerivativeGain_i_idx_1;

      // Sum: '<S386>/Sum' incorporates:
      //   DiscreteIntegrator: '<S377>/Integrator'
      //   DiscreteTransferFcn: '<S370>/Filter Differentiator TF'
      //   Gain: '<S380>/Filter Coefficient'
      //   Gain: '<S382>/Proportional Gain'
      //   Math: '<S370>/Reciprocal'
      //   Product: '<S370>/DenCoefOut'
      //
      //  About '<S370>/Reciprocal':
      //   Operator: reciprocal

      localB->absxk = (rtb_DerivativeGain_i_idx_1 -
                       localDW->FilterDifferentiatorTF_state_hl[1]) *
        localB->Sat_acc_vert * posFilterN + (posP_xy * localB->thrust_sp +
        localDW->Integrator_DSTATE_k[1]);

      // RateTransition: '<S1>/RT_VelocityEst_1kTo50Hz'
      localDW->RT_VelocityEst_1kTo50Hz_Buffer[0] = rtu_StateEstBus->
        Velocity_NED[0];
      localDW->RT_VelocityEst_1kTo50Hz_Buffer[1] = rtu_StateEstBus->
        Velocity_NED[1];
      localDW->RT_VelocityEst_1kTo50Hz_Buffer[2] = rtu_StateEstBus->
        Velocity_NED[2];

      // Math: '<S485>/Reciprocal' incorporates:
      //   Constant: '<S485>/Filter Den Constant'
      //   Constant: '<S494>/N Copy'
      //   SampleTimeMath: '<S487>/Tsamp'
      //   Sum: '<S485>/SumDen'
      //
      //  About '<S485>/Reciprocal':
      //   Operator: reciprocal
      //
      //  About '<S487>/Tsamp':
      //   y = u * K where K = ( w * Ts )
      //
      localB->IntegralGain_l = 1.0F / (velFilterN * 0.02F + 1.0F);

      // Math: '<S541>/Reciprocal'
      //
      //  About '<S541>/Reciprocal':
      //   Operator: reciprocal

      localB->RT_ThrustCmd_50To1kHz = localB->IntegralGain_l;

      // Sum: '<S4>/Sum1' incorporates:
      //   RateTransition: '<S1>/RT_PositionCmd_1kTo50Hz'
      //   RateTransition: '<S1>/RT_PositionEst_1kTo50Hz'

      localB->IntegralGain_m = localDW->RT_PositionCmd_1kTo50Hz_Buffer[2] -
        localDW->RT_PositionEst_1kTo50Hz_Buffer[2];

      // Saturate: '<S384>/Saturation' incorporates:
      //   Sum: '<S386>/Sum'

      if (localB->scale > maxVel_xy) {
        localB->q_norm = maxVel_xy;
      } else if (localB->scale < -10.0F) {
        localB->q_norm = -10.0F;
      } else {
        localB->q_norm = localB->scale;
      }

      // Saturate: '<S4>/Sat_vel_hori'
      if (localB->q_norm > maxVel_xy) {
        localB->q_norm = maxVel_xy;
      } else if (localB->q_norm < -maxVel_xy) {
        localB->q_norm = -maxVel_xy;
      }

      // Sum: '<S5>/Add1' incorporates:
      //   RateTransition: '<S1>/RT_VelocityEst_1kTo50Hz'
      //   Saturate: '<S4>/Sat_vel_hori'

      rtb_DeadZone_g_idx_1 = localB->q_norm -
        localDW->RT_VelocityEst_1kTo50Hz_Buffer[0];

      // DiscreteTransferFcn: '<S485>/Filter Differentiator TF' incorporates:
      //   Gain: '<S483>/Derivative Gain'
      //   UnaryMinus: '<S485>/Unary Minus'

      rtb_DerivativeGain_i_idx_1 = velD_xy * rtb_DeadZone_g_idx_1 -
        -localB->IntegralGain_l * localDW->FilterDifferentiatorTF_states_m[0];
      localDW->FilterDifferentiatorTF_tmp_c[0] = rtb_DerivativeGain_i_idx_1;

      // Sum: '<S501>/Sum' incorporates:
      //   DiscreteIntegrator: '<S492>/Integrator'
      //   DiscreteTransferFcn: '<S485>/Filter Differentiator TF'
      //   Gain: '<S495>/Filter Coefficient'
      //   Gain: '<S497>/Proportional Gain'
      //   Product: '<S485>/DenCoefOut'

      rtb_DeadZone_a_idx_1 = (rtb_DerivativeGain_i_idx_1 -
        localDW->FilterDifferentiatorTF_states_m[0]) * localB->IntegralGain_l *
        velFilterN + (velP_xy * rtb_DeadZone_g_idx_1 +
                      localDW->Integrator_DSTATE_m[0]);

      // Saturate: '<S499>/Saturation'
      if (rtb_DeadZone_a_idx_1 > maxAcc_xy) {
        rtb_DerivativeGain_i_idx_1 = maxAcc_xy;
      } else if (rtb_DeadZone_a_idx_1 < -5.0F) {
        rtb_DerivativeGain_i_idx_1 = -5.0F;
      } else {
        rtb_DerivativeGain_i_idx_1 = rtb_DeadZone_a_idx_1;
      }

      // Saturate: '<S5>/Sat_acc_hori'
      if (rtb_DerivativeGain_i_idx_1 > maxAcc_xy) {
        rtb_DerivativeGain_i_idx_1 = maxAcc_xy;
      } else if (rtb_DerivativeGain_i_idx_1 < -maxAcc_xy) {
        rtb_DerivativeGain_i_idx_1 = -maxAcc_xy;
      }

      rtb_DerivativeGain_i_idx_0 = rtb_DerivativeGain_i_idx_1;

      // DeadZone: '<S482>/DeadZone'
      if (rtb_DeadZone_a_idx_1 > maxAcc_xy) {
        rtb_DeadZone_a_idx_1 -= maxAcc_xy;
      } else if (rtb_DeadZone_a_idx_1 >= -5.0F) {
        rtb_DeadZone_a_idx_1 = 0.0F;
      } else {
        rtb_DeadZone_a_idx_1 -= -5.0F;
      }

      // Gain: '<S489>/Integral Gain'
      rtb_DeadZone_g_idx_1 *= velI_xy;

      // Switch: '<S480>/Switch1' incorporates:
      //   Constant: '<S480>/Clamping_zero'
      //   Constant: '<S480>/Constant'
      //   Constant: '<S480>/Constant2'
      //   RelationalOperator: '<S480>/fix for DT propagation issue'

      if (rtb_DeadZone_a_idx_1 > 0.0F) {
        tmp_1 = 1;
      } else {
        tmp_1 = -1;
      }

      // Switch: '<S480>/Switch2' incorporates:
      //   Constant: '<S480>/Clamping_zero'
      //   Constant: '<S480>/Constant3'
      //   Constant: '<S480>/Constant4'
      //   RelationalOperator: '<S480>/fix for DT propagation issue1'

      if (rtb_DeadZone_g_idx_1 > 0.0F) {
        tmp_2 = 1;
      } else {
        tmp_2 = -1;
      }

      // Switch: '<S480>/Switch' incorporates:
      //   Constant: '<S480>/Clamping_zero'
      //   Constant: '<S480>/Constant1'
      //   Logic: '<S480>/AND3'
      //   RelationalOperator: '<S480>/Equal1'
      //   RelationalOperator: '<S480>/Relational Operator'
      //   Switch: '<S480>/Switch1'
      //   Switch: '<S480>/Switch2'

      if ((rtb_DeadZone_a_idx_1 != 0.0F) && (tmp_1 == tmp_2)) {
        rtb_DeadZone_g_idx_0 = 0.0F;
      } else {
        rtb_DeadZone_g_idx_0 = rtb_DeadZone_g_idx_1;
      }

      // DeadZone: '<S367>/DeadZone' incorporates:
      //   Sum: '<S386>/Sum'

      if (localB->scale > maxVel_xy) {
        localB->scale -= maxVel_xy;
      } else if (localB->scale >= -10.0F) {
        localB->scale = 0.0F;
      } else {
        localB->scale -= -10.0F;
      }

      // Gain: '<S374>/Integral Gain'
      rtb_DeadZone_a_idx_1 = posI_xy * localB->TmpSignalConversionAtFilterDiff;

      // Switch: '<S365>/Switch1' incorporates:
      //   Constant: '<S365>/Clamping_zero'
      //   Constant: '<S365>/Constant'
      //   Constant: '<S365>/Constant2'
      //   RelationalOperator: '<S365>/fix for DT propagation issue'

      if (localB->scale > 0.0F) {
        tmp_1 = 1;
      } else {
        tmp_1 = -1;
      }

      // Switch: '<S365>/Switch2' incorporates:
      //   Constant: '<S365>/Clamping_zero'
      //   Constant: '<S365>/Constant3'
      //   Constant: '<S365>/Constant4'
      //   RelationalOperator: '<S365>/fix for DT propagation issue1'

      if (rtb_DeadZone_a_idx_1 > 0.0F) {
        tmp_2 = 1;
      } else {
        tmp_2 = -1;
      }

      // Switch: '<S365>/Switch' incorporates:
      //   Constant: '<S365>/Clamping_zero'
      //   Constant: '<S365>/Constant1'
      //   Logic: '<S365>/AND3'
      //   RelationalOperator: '<S365>/Equal1'
      //   RelationalOperator: '<S365>/Relational Operator'
      //   Switch: '<S365>/Switch1'
      //   Switch: '<S365>/Switch2'

      if ((localB->scale != 0.0F) && (tmp_1 == tmp_2)) {
        localB->TmpSignalConversionAtFilterDiff = 0.0F;
      } else {
        localB->TmpSignalConversionAtFilterDiff = rtb_DeadZone_a_idx_1;
      }

      // MATLAB Function: '<S5>/CollectiveThrust' incorporates:
      //   Constant: '<S5>/Constant'
      //   SignalConversion generated from: '<S451>/ SFunction '

      localB->RT_RateSP_250To1kHz[0] = 0.0F - rtb_DerivativeGain_i_idx_1;

      // Saturate: '<S384>/Saturation' incorporates:
      //   Sum: '<S386>/Sum'

      if (localB->absxk > maxVel_xy) {
        localB->q_norm = maxVel_xy;
      } else if (localB->absxk < -10.0F) {
        localB->q_norm = -10.0F;
      } else {
        localB->q_norm = localB->absxk;
      }

      // Saturate: '<S4>/Sat_vel_hori'
      if (localB->q_norm > maxVel_xy) {
        localB->q_norm = maxVel_xy;
      } else if (localB->q_norm < -maxVel_xy) {
        localB->q_norm = -maxVel_xy;
      }

      // Sum: '<S5>/Add1' incorporates:
      //   RateTransition: '<S1>/RT_VelocityEst_1kTo50Hz'
      //   Saturate: '<S4>/Sat_vel_hori'

      rtb_DeadZone_g_idx_1 = localB->q_norm -
        localDW->RT_VelocityEst_1kTo50Hz_Buffer[1];

      // DiscreteTransferFcn: '<S485>/Filter Differentiator TF' incorporates:
      //   Gain: '<S483>/Derivative Gain'
      //   UnaryMinus: '<S485>/Unary Minus'

      rtb_DerivativeGain_i_idx_1 = velD_xy * rtb_DeadZone_g_idx_1 -
        -localB->IntegralGain_l * localDW->FilterDifferentiatorTF_states_m[1];
      localDW->FilterDifferentiatorTF_tmp_c[1] = rtb_DerivativeGain_i_idx_1;

      // Sum: '<S501>/Sum' incorporates:
      //   DiscreteIntegrator: '<S492>/Integrator'
      //   DiscreteTransferFcn: '<S485>/Filter Differentiator TF'
      //   Gain: '<S495>/Filter Coefficient'
      //   Gain: '<S497>/Proportional Gain'
      //   Product: '<S485>/DenCoefOut'

      rtb_DeadZone_a_idx_1 = (rtb_DerivativeGain_i_idx_1 -
        localDW->FilterDifferentiatorTF_states_m[1]) * localB->IntegralGain_l *
        velFilterN + (velP_xy * rtb_DeadZone_g_idx_1 +
                      localDW->Integrator_DSTATE_m[1]);

      // Saturate: '<S499>/Saturation'
      if (rtb_DeadZone_a_idx_1 > maxAcc_xy) {
        rtb_DerivativeGain_i_idx_1 = maxAcc_xy;
      } else if (rtb_DeadZone_a_idx_1 < -5.0F) {
        rtb_DerivativeGain_i_idx_1 = -5.0F;
      } else {
        rtb_DerivativeGain_i_idx_1 = rtb_DeadZone_a_idx_1;
      }

      // Saturate: '<S5>/Sat_acc_hori'
      if (rtb_DerivativeGain_i_idx_1 > maxAcc_xy) {
        rtb_DerivativeGain_i_idx_1 = maxAcc_xy;
      } else if (rtb_DerivativeGain_i_idx_1 < -maxAcc_xy) {
        rtb_DerivativeGain_i_idx_1 = -maxAcc_xy;
      }

      // DeadZone: '<S482>/DeadZone'
      if (rtb_DeadZone_a_idx_1 > maxAcc_xy) {
        rtb_DeadZone_a_idx_1 -= maxAcc_xy;
      } else if (rtb_DeadZone_a_idx_1 >= -5.0F) {
        rtb_DeadZone_a_idx_1 = 0.0F;
      } else {
        rtb_DeadZone_a_idx_1 -= -5.0F;
      }

      // Gain: '<S489>/Integral Gain'
      rtb_DeadZone_g_idx_1 *= velI_xy;

      // Switch: '<S480>/Switch1' incorporates:
      //   Constant: '<S480>/Clamping_zero'
      //   Constant: '<S480>/Constant'
      //   Constant: '<S480>/Constant2'
      //   RelationalOperator: '<S480>/fix for DT propagation issue'

      if (rtb_DeadZone_a_idx_1 > 0.0F) {
        tmp_1 = 1;
      } else {
        tmp_1 = -1;
      }

      // Switch: '<S480>/Switch2' incorporates:
      //   Constant: '<S480>/Clamping_zero'
      //   Constant: '<S480>/Constant3'
      //   Constant: '<S480>/Constant4'
      //   RelationalOperator: '<S480>/fix for DT propagation issue1'

      if (rtb_DeadZone_g_idx_1 > 0.0F) {
        tmp_2 = 1;
      } else {
        tmp_2 = -1;
      }

      // Switch: '<S480>/Switch' incorporates:
      //   Constant: '<S480>/Clamping_zero'
      //   Constant: '<S480>/Constant1'
      //   Logic: '<S480>/AND3'
      //   RelationalOperator: '<S480>/Equal1'
      //   RelationalOperator: '<S480>/Relational Operator'
      //   Switch: '<S480>/Switch1'
      //   Switch: '<S480>/Switch2'

      if ((rtb_DeadZone_a_idx_1 != 0.0F) && (tmp_1 == tmp_2)) {
        rtb_DeadZone_g_idx_1 = 0.0F;
      }

      // DeadZone: '<S367>/DeadZone' incorporates:
      //   Sum: '<S386>/Sum'

      if (localB->absxk > maxVel_xy) {
        localB->scale = localB->absxk - maxVel_xy;
      } else if (localB->absxk >= -10.0F) {
        localB->scale = 0.0F;
      } else {
        localB->scale = localB->absxk - -10.0F;
      }

      // Gain: '<S374>/Integral Gain' incorporates:
      //   Sum: '<S4>/Sum'

      rtb_DeadZone_a_idx_1 = posI_xy * localB->thrust_sp;

      // Switch: '<S365>/Switch1' incorporates:
      //   Constant: '<S365>/Clamping_zero'
      //   Constant: '<S365>/Constant'
      //   Constant: '<S365>/Constant2'
      //   RelationalOperator: '<S365>/fix for DT propagation issue'

      if (localB->scale > 0.0F) {
        tmp_1 = 1;
      } else {
        tmp_1 = -1;
      }

      // Switch: '<S365>/Switch2' incorporates:
      //   Constant: '<S365>/Clamping_zero'
      //   Constant: '<S365>/Constant3'
      //   Constant: '<S365>/Constant4'
      //   RelationalOperator: '<S365>/fix for DT propagation issue1'

      if (rtb_DeadZone_a_idx_1 > 0.0F) {
        tmp_2 = 1;
      } else {
        tmp_2 = -1;
      }

      // Switch: '<S365>/Switch' incorporates:
      //   Constant: '<S365>/Clamping_zero'
      //   Constant: '<S365>/Constant1'
      //   Logic: '<S365>/AND3'
      //   RelationalOperator: '<S365>/Equal1'
      //   RelationalOperator: '<S365>/Relational Operator'
      //   Switch: '<S365>/Switch1'
      //   Switch: '<S365>/Switch2'

      if ((localB->scale != 0.0F) && (tmp_1 == tmp_2)) {
        rtb_DeadZone_a_idx_1 = 0.0F;
      }

      // MATLAB Function: '<S5>/CollectiveThrust' incorporates:
      //   Constant: '<S5>/Constant'
      //   SignalConversion generated from: '<S451>/ SFunction '

      localB->RT_RateSP_250To1kHz[1] = 0.0F - rtb_DerivativeGain_i_idx_1;

      // DiscreteTransferFcn: '<S426>/Filter Differentiator TF' incorporates:
      //   Gain: '<S424>/Derivative Gain'
      //   UnaryMinus: '<S426>/Unary Minus'

      localDW->FilterDifferentiatorTF_tmp_p = posD_z * localB->IntegralGain_m -
        -localB->Sat_acc_vert * localDW->FilterDifferentiatorTF_states_a;

      // Sum: '<S442>/Sum' incorporates:
      //   DiscreteIntegrator: '<S433>/Integrator'
      //   DiscreteTransferFcn: '<S426>/Filter Differentiator TF'
      //   Gain: '<S436>/Filter Coefficient'
      //   Gain: '<S438>/Proportional Gain'
      //   Product: '<S426>/DenCoefOut'

      localB->DeadZone_gb = (localDW->FilterDifferentiatorTF_tmp_p -
        localDW->FilterDifferentiatorTF_states_a) * localB->Sat_acc_vert *
        posFilterN + (posP_z * localB->IntegralGain_m +
                      localDW->Integrator_DSTATE_o);

      // Saturate: '<S440>/Saturation'
      if (localB->DeadZone_gb > maxVel_z) {
        localB->q_norm = maxVel_z;
      } else if (localB->DeadZone_gb < -5.0F) {
        localB->q_norm = -5.0F;
      } else {
        localB->q_norm = localB->DeadZone_gb;
      }

      // End of Saturate: '<S440>/Saturation'

      // Saturate: '<S4>/Sat_vel_vert'
      if (localB->q_norm > maxVel_z) {
        localB->q_norm = maxVel_z;
      } else if (localB->q_norm < -maxVel_z) {
        localB->q_norm = -maxVel_z;
      }

      // Sum: '<S5>/Add' incorporates:
      //   RateTransition: '<S1>/RT_VelocityEst_1kTo50Hz'
      //   Saturate: '<S4>/Sat_vel_vert'

      localB->IntegralGain_l = localB->q_norm -
        localDW->RT_VelocityEst_1kTo50Hz_Buffer[2];

      // DiscreteTransferFcn: '<S541>/Filter Differentiator TF' incorporates:
      //   Gain: '<S539>/Derivative Gain'
      //   UnaryMinus: '<S541>/Unary Minus'

      localDW->FilterDifferentiatorTF_tmp_m = velD_z * localB->IntegralGain_l -
        -localB->RT_ThrustCmd_50To1kHz *
        localDW->FilterDifferentiatorTF_states_e;

      // Sum: '<S557>/Sum' incorporates:
      //   DiscreteIntegrator: '<S548>/Integrator'
      //   DiscreteTransferFcn: '<S541>/Filter Differentiator TF'
      //   Gain: '<S551>/Filter Coefficient'
      //   Gain: '<S553>/Proportional Gain'
      //   Product: '<S541>/DenCoefOut'

      localB->RT_ThrustCmd_50To1kHz = (localDW->FilterDifferentiatorTF_tmp_m -
        localDW->FilterDifferentiatorTF_states_e) *
        localB->RT_ThrustCmd_50To1kHz * velFilterN + (velP_z *
        localB->IntegralGain_l + localDW->Integrator_DSTATE_p);

      // Saturate: '<S555>/Saturation'
      if (localB->RT_ThrustCmd_50To1kHz > maxAcc_z) {
        localB->Sat_acc_vert = maxAcc_z;
      } else if (localB->RT_ThrustCmd_50To1kHz < -10.0F) {
        localB->Sat_acc_vert = -10.0F;
      } else {
        localB->Sat_acc_vert = localB->RT_ThrustCmd_50To1kHz;
      }

      // End of Saturate: '<S555>/Saturation'

      // Saturate: '<S5>/Sat_acc_vert'
      if (localB->Sat_acc_vert > maxAcc_z) {
        localB->Sat_acc_vert = maxAcc_z;
      } else if (localB->Sat_acc_vert < -maxAcc_z) {
        localB->Sat_acc_vert = -maxAcc_z;
      }

      // End of Saturate: '<S5>/Sat_acc_vert'

      // MATLAB Function: '<S5>/CollectiveThrust' incorporates:
      //   Constant: '<S5>/Constant'
      //   Constant: '<S5>/Constant1'
      //   SignalConversion generated from: '<S451>/ SFunction '

      localB->RT_RateSP_250To1kHz[2] = 9.80665F - localB->Sat_acc_vert;
      localB->scale = 1.29246971E-26F;
      localB->absxk = std::abs(localB->RT_RateSP_250To1kHz[0]);
      if (localB->absxk > 1.29246971E-26F) {
        localB->q_norm = 1.0F;
        localB->scale = localB->absxk;
      } else {
        localB->t = localB->absxk / 1.29246971E-26F;
        localB->q_norm = localB->t * localB->t;
      }

      localB->absxk = std::abs(localB->RT_RateSP_250To1kHz[1]);
      if (localB->absxk > localB->scale) {
        localB->t = localB->scale / localB->absxk;
        localB->q_norm = localB->q_norm * localB->t * localB->t + 1.0F;
        localB->scale = localB->absxk;
      } else {
        localB->t = localB->absxk / localB->scale;
        localB->q_norm += localB->t * localB->t;
      }

      localB->absxk = std::abs(localB->RT_RateSP_250To1kHz[2]);
      if (localB->absxk > localB->scale) {
        localB->t = localB->scale / localB->absxk;
        localB->q_norm = localB->q_norm * localB->t * localB->t + 1.0F;
        localB->scale = localB->absxk;
      } else {
        localB->t = localB->absxk / localB->scale;
        localB->q_norm += localB->t * localB->t;
      }

      localB->thrust_sp = localB->scale * std::sqrt(localB->q_norm) * mass;

      // DeadZone: '<S538>/DeadZone'
      if (localB->RT_ThrustCmd_50To1kHz > maxAcc_z) {
        localB->RT_ThrustCmd_50To1kHz -= maxAcc_z;
      } else if (localB->RT_ThrustCmd_50To1kHz >= -10.0F) {
        localB->RT_ThrustCmd_50To1kHz = 0.0F;
      } else {
        localB->RT_ThrustCmd_50To1kHz -= -10.0F;
      }

      // End of DeadZone: '<S538>/DeadZone'

      // Gain: '<S545>/Integral Gain'
      localB->IntegralGain_l *= velI_z;

      // Switch: '<S536>/Switch1' incorporates:
      //   Constant: '<S536>/Clamping_zero'
      //   Constant: '<S536>/Constant'
      //   Constant: '<S536>/Constant2'
      //   RelationalOperator: '<S536>/fix for DT propagation issue'

      if (localB->RT_ThrustCmd_50To1kHz > 0.0F) {
        tmp_1 = 1;
      } else {
        tmp_1 = -1;
      }

      // Switch: '<S536>/Switch2' incorporates:
      //   Constant: '<S536>/Clamping_zero'
      //   Constant: '<S536>/Constant3'
      //   Constant: '<S536>/Constant4'
      //   RelationalOperator: '<S536>/fix for DT propagation issue1'

      if (localB->IntegralGain_l > 0.0F) {
        tmp_2 = 1;
      } else {
        tmp_2 = -1;
      }

      // Switch: '<S536>/Switch' incorporates:
      //   Constant: '<S536>/Clamping_zero'
      //   Constant: '<S536>/Constant1'
      //   Logic: '<S536>/AND3'
      //   RelationalOperator: '<S536>/Equal1'
      //   RelationalOperator: '<S536>/Relational Operator'
      //   Switch: '<S536>/Switch1'
      //   Switch: '<S536>/Switch2'

      if ((localB->RT_ThrustCmd_50To1kHz != 0.0F) && (tmp_1 == tmp_2)) {
        localB->IntegralGain_l = 0.0F;
      }

      // End of Switch: '<S536>/Switch'

      // DeadZone: '<S423>/DeadZone'
      if (localB->DeadZone_gb > maxVel_z) {
        localB->DeadZone_gb -= maxVel_z;
      } else if (localB->DeadZone_gb >= -5.0F) {
        localB->DeadZone_gb = 0.0F;
      } else {
        localB->DeadZone_gb -= -5.0F;
      }

      // End of DeadZone: '<S423>/DeadZone'

      // Gain: '<S430>/Integral Gain'
      localB->IntegralGain_m *= posI_z;

      // Switch: '<S421>/Switch1' incorporates:
      //   Constant: '<S421>/Clamping_zero'
      //   Constant: '<S421>/Constant'
      //   Constant: '<S421>/Constant2'
      //   RelationalOperator: '<S421>/fix for DT propagation issue'

      if (localB->DeadZone_gb > 0.0F) {
        tmp_1 = 1;
      } else {
        tmp_1 = -1;
      }

      // Switch: '<S421>/Switch2' incorporates:
      //   Constant: '<S421>/Clamping_zero'
      //   Constant: '<S421>/Constant3'
      //   Constant: '<S421>/Constant4'
      //   RelationalOperator: '<S421>/fix for DT propagation issue1'

      if (localB->IntegralGain_m > 0.0F) {
        tmp_2 = 1;
      } else {
        tmp_2 = -1;
      }

      // Switch: '<S421>/Switch' incorporates:
      //   Constant: '<S421>/Clamping_zero'
      //   Constant: '<S421>/Constant1'
      //   Logic: '<S421>/AND3'
      //   RelationalOperator: '<S421>/Equal1'
      //   RelationalOperator: '<S421>/Relational Operator'
      //   Switch: '<S421>/Switch1'
      //   Switch: '<S421>/Switch2'

      if ((localB->DeadZone_gb != 0.0F) && (tmp_1 == tmp_2)) {
        localB->IntegralGain_m = 0.0F;
      }

      // End of Switch: '<S421>/Switch'
    }

    // Update for RateTransition: '<S2>/RT_RateSP_250To1kHz'
    if (tmp) {
      localDW->RT_RateSP_250To1kHz_Buffer
        [(localDW->RT_RateSP_250To1kHz_ActiveBufId == 0) * 3] = localB->q_idx_1;
      localDW->RT_RateSP_250To1kHz_Buffer[1 +
        (localDW->RT_RateSP_250To1kHz_ActiveBufId == 0) * 3] = q_idx_0;
      localDW->RT_RateSP_250To1kHz_Buffer[2 +
        (localDW->RT_RateSP_250To1kHz_ActiveBufId == 0) * 3] = localB->q_idx_2;
      localDW->RT_RateSP_250To1kHz_ActiveBufId = static_cast<int8_T>
        (localDW->RT_RateSP_250To1kHz_ActiveBufId == 0);
    }

    // Switch: '<S91>/Switch1' incorporates:
    //   Constant: '<S91>/Clamping_zero'
    //   Constant: '<S91>/Constant'
    //   Constant: '<S91>/Constant2'
    //   RelationalOperator: '<S91>/fix for DT propagation issue'

    if (localB->DeadZone > 0.0F) {
      tmp_1 = 1;
    } else {
      tmp_1 = -1;
    }

    // Switch: '<S91>/Switch2' incorporates:
    //   Constant: '<S91>/Clamping_zero'
    //   Constant: '<S91>/Constant3'
    //   Constant: '<S91>/Constant4'
    //   RelationalOperator: '<S91>/fix for DT propagation issue1'

    if (localB->IntegralGain > 0.0F) {
      tmp_2 = 1;
    } else {
      tmp_2 = -1;
    }

    // Switch: '<S91>/Switch' incorporates:
    //   Constant: '<S91>/Clamping_zero'
    //   Constant: '<S91>/Constant1'
    //   Logic: '<S91>/AND3'
    //   RelationalOperator: '<S91>/Equal1'
    //   RelationalOperator: '<S91>/Relational Operator'
    //   Switch: '<S91>/Switch1'
    //   Switch: '<S91>/Switch2'

    if ((localB->DeadZone != 0.0F) && (tmp_1 == tmp_2)) {
      localB->IntegralGain = 0.0F;
    }

    // Update for DiscreteIntegrator: '<S103>/Integrator' incorporates:
    //   Switch: '<S91>/Switch'

    localDW->Integrator_DSTATE += 0.001F * localB->IntegralGain;

    // Update for DiscreteTransferFcn: '<S96>/Filter Differentiator TF'
    localDW->FilterDifferentiatorTF_states = localDW->FilterDifferentiatorTF_tmp;

    // Switch: '<S147>/Switch1' incorporates:
    //   Constant: '<S147>/Clamping_zero'
    //   Constant: '<S147>/Constant'
    //   Constant: '<S147>/Constant2'
    //   RelationalOperator: '<S147>/fix for DT propagation issue'

    if (localB->DeadZone_i > 0.0F) {
      tmp_1 = 1;
    } else {
      tmp_1 = -1;
    }

    // Switch: '<S147>/Switch2' incorporates:
    //   Constant: '<S147>/Clamping_zero'
    //   Constant: '<S147>/Constant3'
    //   Constant: '<S147>/Constant4'
    //   RelationalOperator: '<S147>/fix for DT propagation issue1'

    if (localB->IntegralGain_f > 0.0F) {
      tmp_2 = 1;
    } else {
      tmp_2 = -1;
    }

    // Switch: '<S147>/Switch' incorporates:
    //   Constant: '<S147>/Clamping_zero'
    //   Constant: '<S147>/Constant1'
    //   Logic: '<S147>/AND3'
    //   RelationalOperator: '<S147>/Equal1'
    //   RelationalOperator: '<S147>/Relational Operator'
    //   Switch: '<S147>/Switch1'
    //   Switch: '<S147>/Switch2'

    if ((localB->DeadZone_i != 0.0F) && (tmp_1 == tmp_2)) {
      localB->IntegralGain_f = 0.0F;
    }

    // Update for DiscreteIntegrator: '<S159>/Integrator' incorporates:
    //   Switch: '<S147>/Switch'

    localDW->Integrator_DSTATE_g += 0.001F * localB->IntegralGain_f;

    // Update for DiscreteTransferFcn: '<S152>/Filter Differentiator TF'
    localDW->FilterDifferentiatorTF_states_h =
      localDW->FilterDifferentiatorTF_tmp_b;

    // Switch: '<S203>/Switch1' incorporates:
    //   Constant: '<S203>/Clamping_zero'
    //   Constant: '<S203>/Constant'
    //   Constant: '<S203>/Constant2'
    //   RelationalOperator: '<S203>/fix for DT propagation issue'

    if (localB->DeadZone_o > 0.0F) {
      tmp_1 = 1;
    } else {
      tmp_1 = -1;
    }

    // Switch: '<S203>/Switch2' incorporates:
    //   Constant: '<S203>/Clamping_zero'
    //   Constant: '<S203>/Constant3'
    //   Constant: '<S203>/Constant4'
    //   RelationalOperator: '<S203>/fix for DT propagation issue1'

    if (localB->IntegralGain_h > 0.0F) {
      tmp_2 = 1;
    } else {
      tmp_2 = -1;
    }

    // Switch: '<S203>/Switch' incorporates:
    //   Constant: '<S203>/Clamping_zero'
    //   Constant: '<S203>/Constant1'
    //   Logic: '<S203>/AND3'
    //   RelationalOperator: '<S203>/Equal1'
    //   RelationalOperator: '<S203>/Relational Operator'
    //   Switch: '<S203>/Switch1'
    //   Switch: '<S203>/Switch2'

    if ((localB->DeadZone_o != 0.0F) && (tmp_1 == tmp_2)) {
      localB->IntegralGain_h = 0.0F;
    }

    // Update for DiscreteIntegrator: '<S215>/Integrator' incorporates:
    //   Switch: '<S203>/Switch'

    localDW->Integrator_DSTATE_a += 0.001F * localB->IntegralGain_h;

    // Update for DiscreteTransferFcn: '<S208>/Filter Differentiator TF'
    localDW->FilterDifferentiatorTF_states_l =
      localDW->FilterDifferentiatorTF_tmp_g;

    // Update for RateTransition: '<S1>/RT_ThrustCmd_50To1kHz'
    if (tmp_0) {
      localDW->RT_ThrustCmd_50To1kHz_Buffer0 = localB->thrust_sp;

      // Update for RateTransition: '<S1>/RT_AccCmd_50To250Hz'
      localDW->RT_AccCmd_50To250Hz_Buffer
        [(localDW->RT_AccCmd_50To250Hz_ActiveBufId == 0) * 3] =
        rtb_DerivativeGain_i_idx_0;
      localDW->RT_AccCmd_50To250Hz_Buffer[1 +
        (localDW->RT_AccCmd_50To250Hz_ActiveBufId == 0) * 3] =
        rtb_DerivativeGain_i_idx_1;
      localDW->RT_AccCmd_50To250Hz_Buffer[2 +
        (localDW->RT_AccCmd_50To250Hz_ActiveBufId == 0) * 3] =
        localB->Sat_acc_vert;
      localDW->RT_AccCmd_50To250Hz_ActiveBufId = static_cast<int8_T>
        (localDW->RT_AccCmd_50To250Hz_ActiveBufId == 0);

      // Update for DiscreteIntegrator: '<S377>/Integrator'
      localDW->Integrator_DSTATE_k[0] += 0.02F *
        localB->TmpSignalConversionAtFilterDiff;

      // Update for DiscreteTransferFcn: '<S370>/Filter Differentiator TF'
      localDW->FilterDifferentiatorTF_state_hl[0] =
        localDW->FilterDifferentiatorTF_tmp_j[0];

      // Update for DiscreteIntegrator: '<S492>/Integrator'
      localDW->Integrator_DSTATE_m[0] += 0.02F * rtb_DeadZone_g_idx_0;

      // Update for DiscreteTransferFcn: '<S485>/Filter Differentiator TF'
      localDW->FilterDifferentiatorTF_states_m[0] =
        localDW->FilterDifferentiatorTF_tmp_c[0];

      // Update for DiscreteIntegrator: '<S377>/Integrator'
      localDW->Integrator_DSTATE_k[1] += 0.02F * rtb_DeadZone_a_idx_1;

      // Update for DiscreteTransferFcn: '<S370>/Filter Differentiator TF'
      localDW->FilterDifferentiatorTF_state_hl[1] =
        localDW->FilterDifferentiatorTF_tmp_j[1];

      // Update for DiscreteIntegrator: '<S492>/Integrator'
      localDW->Integrator_DSTATE_m[1] += 0.02F * rtb_DeadZone_g_idx_1;

      // Update for DiscreteTransferFcn: '<S485>/Filter Differentiator TF'
      localDW->FilterDifferentiatorTF_states_m[1] =
        localDW->FilterDifferentiatorTF_tmp_c[1];

      // Update for DiscreteIntegrator: '<S433>/Integrator'
      localDW->Integrator_DSTATE_o += 0.02F * localB->IntegralGain_m;

      // Update for DiscreteTransferFcn: '<S426>/Filter Differentiator TF'
      localDW->FilterDifferentiatorTF_states_a =
        localDW->FilterDifferentiatorTF_tmp_p;

      // Update for DiscreteTransferFcn: '<S541>/Filter Differentiator TF'
      localDW->FilterDifferentiatorTF_states_e =
        localDW->FilterDifferentiatorTF_tmp_m;

      // Update for DiscreteIntegrator: '<S548>/Integrator'
      localDW->Integrator_DSTATE_p += 0.02F * localB->IntegralGain_l;
    }
  } else if (localDW->ControlLaw_MODE) {
    // Disable for Outport: '<S1>/MotorCmd'
    localB->motor_cmd[0] = 0.0F;
    localB->motor_cmd[1] = 0.0F;
    localB->motor_cmd[2] = 0.0F;
    localB->motor_cmd[3] = 0.0F;
    localDW->ControlLaw_MODE = false;
  }

  // End of Outputs for SubSystem: '<Root>/ControlLaw'

  // BusCreator: '<Root>/Bus Creator'
  rty_EscCmdBus->MotorCmd[0] = localB->motor_cmd[0];
  rty_EscCmdBus->MotorCmd[1] = localB->motor_cmd[1];
  rty_EscCmdBus->MotorCmd[2] = localB->motor_cmd[2];
  rty_EscCmdBus->MotorCmd[3] = localB->motor_cmd[3];
  rty_EscCmdBus->Armed = *rtu_Armed;
  rty_EscCmdBus->Valid = *rtu_ControlActive;
}

// Model initialize function
void UAV_FlightControl_initialize(const char_T **rt_errorStatus, const
  rtTimingBridge *timingBridge, int_T mdlref_TID0, int_T mdlref_TID1, int_T
  mdlref_TID2, RT_MODEL_UAV_FlightControl_T *const UAV_FlightControl_M)
{
  // Registration code

  // setup the global timing engine
  UAV_FlightControl_M->Timing.mdlref_GlobalTID[0] = mdlref_TID0;
  UAV_FlightControl_M->Timing.mdlref_GlobalTID[1] = mdlref_TID1;
  UAV_FlightControl_M->Timing.mdlref_GlobalTID[2] = mdlref_TID2;
  UAV_FlightControl_M->timingBridge = (timingBridge);

  // initialize error status
  UAV_FlightControl_M->setErrorStatusPointer(rt_errorStatus);
}

time_T RT_MODEL_UAV_FlightControl_T::getClockTickH0() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[0]]) );
}

time_T RT_MODEL_UAV_FlightControl_T::getClockTick0() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[0]])) );
}

time_T RT_MODEL_UAV_FlightControl_T::getClockTickH1() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[1]]) );
}

time_T RT_MODEL_UAV_FlightControl_T::getClockTick1() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[1]])) );
}

time_T RT_MODEL_UAV_FlightControl_T::getClockTickH2() const
{
  return ( *(timingBridge->clockTickH[Timing.mdlref_GlobalTID[2]]) );
}

time_T RT_MODEL_UAV_FlightControl_T::getClockTick2() const
{
  return ( *((timingBridge->clockTick[Timing.mdlref_GlobalTID[2]])) );
}

const char_T* RT_MODEL_UAV_FlightControl_T::getErrorStatus() const
{
  return (*(errorStatus));
}

void RT_MODEL_UAV_FlightControl_T::setErrorStatus(const char_T* const
  aErrorStatus) const
{
  (*(errorStatus) = aErrorStatus);
}

const char_T** RT_MODEL_UAV_FlightControl_T::getErrorStatusPointer() const
{
  return errorStatus;
}

void RT_MODEL_UAV_FlightControl_T::setErrorStatusPointer(const char_T
  ** aErrorStatusPointer)
{
  (errorStatus = aErrorStatusPointer);
}

boolean_T RT_MODEL_UAV_FlightControl_T::isSampleHit(int32_T sti) const
{
  return (timingBridge->taskCounter[Timing.mdlref_GlobalTID[sti]] == 0);
}

time_T RT_MODEL_UAV_FlightControl_T::getT() const
{
  return (*(timingBridge->taskTime[0]));
}

//
// File trailer for generated code.
//
// [EOF]
//
