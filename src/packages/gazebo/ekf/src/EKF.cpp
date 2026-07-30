//
// File: EKF.cpp
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
#include "EKF.h"
#include "rtwtypes.h"
#include <cstring>
#include <emmintrin.h>
#include <cmath>
#include <xmmintrin.h>
#include <cfloat>
#include "EKF_private.h"
#include "cmath"

// Forward declaration for local functions
static real_T EKF_rt_remd_snf(real_T u0, real_T u1, B_EKF_c_T *localB);
static void EKF_cosd_ZK11uGDT(real_T *x, B_EKF_c_T *localB);
static void EKF_sind_CjVy6Jw4(real_T *x, B_EKF_c_T *localB);
static void EKF_lla2ecef_WvPbxdnX(const real_T llaPos[3], real_T ecefPos[3],
  B_EKF_c_T *localB);
static real_T EKF_rt_remd_snf(real_T u0, real_T u1, B_EKF_c_T *localB)
{
  real_T y;
  if (std::isnan(u0) || std::isnan(u1) || std::isinf(u0)) {
    y = (rtNaN);
  } else if (std::isinf(u1)) {
    y = u0;
  } else if ((u1 != 0.0) && (u1 != std::trunc(u1))) {
    localB->q = std::abs(u0 / u1);
    if (!(std::abs(localB->q - std::floor(localB->q + 0.5)) > DBL_EPSILON *
          localB->q)) {
      y = 0.0 * u0;
    } else {
      y = std::fmod(u0, u1);
    }
  } else {
    y = std::fmod(u0, u1);
  }

  return y;
}

// Function for MATLAB Function: '<S3>/MATLAB Function'
static void EKF_cosd_ZK11uGDT(real_T *x, B_EKF_c_T *localB)
{
  int8_T n;
  if (std::isinf(*x) || std::isnan(*x)) {
    *x = (rtNaN);
  } else {
    localB->b_x_f = EKF_rt_remd_snf(*x, 360.0, localB);
    localB->absx_g = std::abs(localB->b_x_f);
    if (localB->absx_g > 180.0) {
      if (localB->b_x_f > 0.0) {
        localB->b_x_f -= 360.0;
      } else {
        localB->b_x_f += 360.0;
      }

      localB->absx_g = std::abs(localB->b_x_f);
    }

    if (localB->absx_g <= 45.0) {
      localB->b_x_f *= 0.017453292519943295;
      n = 0;
    } else if (localB->absx_g <= 135.0) {
      if (localB->b_x_f > 0.0) {
        localB->b_x_f = (localB->b_x_f - 90.0) * 0.017453292519943295;
        n = 1;
      } else {
        localB->b_x_f = (localB->b_x_f + 90.0) * 0.017453292519943295;
        n = -1;
      }
    } else if (localB->b_x_f > 0.0) {
      localB->b_x_f = (localB->b_x_f - 180.0) * 0.017453292519943295;
      n = 2;
    } else {
      localB->b_x_f = (localB->b_x_f + 180.0) * 0.017453292519943295;
      n = -2;
    }

    switch (n) {
     case 0:
      *x = std::cos(localB->b_x_f);
      break;

     case 1:
      *x = -std::sin(localB->b_x_f);
      break;

     case -1:
      *x = std::sin(localB->b_x_f);
      break;

     default:
      *x = -std::cos(localB->b_x_f);
      break;
    }
  }
}

// Function for MATLAB Function: '<S3>/MATLAB Function'
static void EKF_sind_CjVy6Jw4(real_T *x, B_EKF_c_T *localB)
{
  int8_T n;
  if (std::isinf(*x) || std::isnan(*x)) {
    *x = (rtNaN);
  } else {
    localB->b_x = EKF_rt_remd_snf(*x, 360.0, localB);
    localB->absx = std::abs(localB->b_x);
    if (localB->absx > 180.0) {
      if (localB->b_x > 0.0) {
        localB->b_x -= 360.0;
      } else {
        localB->b_x += 360.0;
      }

      localB->absx = std::abs(localB->b_x);
    }

    if (localB->absx <= 45.0) {
      localB->b_x *= 0.017453292519943295;
      n = 0;
    } else if (localB->absx <= 135.0) {
      if (localB->b_x > 0.0) {
        localB->b_x = (localB->b_x - 90.0) * 0.017453292519943295;
        n = 1;
      } else {
        localB->b_x = (localB->b_x + 90.0) * 0.017453292519943295;
        n = -1;
      }
    } else if (localB->b_x > 0.0) {
      localB->b_x = (localB->b_x - 180.0) * 0.017453292519943295;
      n = 2;
    } else {
      localB->b_x = (localB->b_x + 180.0) * 0.017453292519943295;
      n = -2;
    }

    switch (n) {
     case 0:
      *x = std::sin(localB->b_x);
      break;

     case 1:
      *x = std::cos(localB->b_x);
      break;

     case -1:
      *x = -std::cos(localB->b_x);
      break;

     default:
      *x = -std::sin(localB->b_x);
      break;
    }
  }
}

// Function for MATLAB Function: '<S3>/MATLAB Function'
static void EKF_lla2ecef_WvPbxdnX(const real_T llaPos[3], real_T ecefPos[3],
  B_EKF_c_T *localB)
{
  localB->sinphi_c = llaPos[0];
  EKF_sind_CjVy6Jw4(&localB->sinphi_c, localB);
  localB->N = 6.378137E+6 / std::sqrt(1.0 - localB->sinphi_c * localB->sinphi_c *
    0.0066943799901413165);
  localB->b = llaPos[0];
  EKF_cosd_ZK11uGDT(&localB->b, localB);
  localB->b *= localB->N + llaPos[2];
  localB->c = llaPos[1];
  EKF_cosd_ZK11uGDT(&localB->c, localB);
  localB->d = llaPos[1];
  EKF_sind_CjVy6Jw4(&localB->d, localB);
  ecefPos[0] = localB->b * localB->c;
  ecefPos[1] = localB->b * localB->d;
  ecefPos[2] = (localB->N * 0.99330562000985867 + llaPos[2]) * localB->sinphi_c;
}

// System initialize for referenced model: 'EKF'
void EKF_Init(DW_EKF_f_T *localDW)
{
  // InitializeConditions for UnitDelay: '<Root>/Unit Delay1'
  std::memcpy(&localDW->UnitDelay1_DSTATE[0], &rtCP_UnitDelay1_InitialConditio[0],
              225U * sizeof(real32_T));

  // InitializeConditions for UnitDelay: '<Root>/Unit Delay'
  std::memcpy(&localDW->UnitDelay_DSTATE[0], &SE_EKF_INIT_STATE[0], sizeof
              (real32_T) << 4U);
}

// Output and update for referenced model: 'EKF'
void EKF(const real32_T rtu_IMU_BUS_Accel[3], const real32_T rtu_IMU_BUS_Gyro[3],
         const boolean_T *rtu_IMU_BUS_Valid, const real32_T *rtu_GPS_BUS_Lat,
         const real32_T *rtu_GPS_BUS_Lon, const real32_T *rtu_GPS_BUS_Alt, const
         real32_T rtu_GPS_BUS_Velocity[3], const boolean_T *rtu_GPS_BUS_Valid,
         const boolean_T *rtu_GPS_BUS_IsNew, real32_T
         rty_StateEstBus_Position_NED[3], real32_T rty_StateEstBus_Velocity_NED
         [3], real32_T rty_StateEstBus_Attitude_quat[4], real32_T
         rty_StateEstBus_AngularRate_Bod[3], real32_T
         rty_StateEstBus_Accel_Body[3], real32_T rty_StateEstBus_GyroBias[3],
         real32_T rty_StateEstBus_AccelBias[3], real32_T
         rty_StateEstBus_Wind_NED[3], uint8_T *rty_StateEstBus_Status, B_EKF_c_T
         *localB, DW_EKF_f_T *localDW)
{
  __m128 tmp;
  __m128 tmp_0;
  __m128d tmp_1;
  int32_T P_p_tmp;
  int32_T a_tmp;
  int32_T a_tmp_0;
  int8_T ipiv;
  boolean_T rtb_GpsUpdateEnable;
  static const int8_T e[225]{ 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
  };

  // MATLAB Function: '<Root>/MATLAB Function' incorporates:
  //   BusCreator generated from: '<Root>/MATLAB Function'
  //   Constant: '<Root>/ekf_predict_dt'
  //   Constant: '<Root>/g_n'
  //   Constant: '<Root>/sigma_acc_c'
  //   Constant: '<Root>/sigma_ba_c'
  //   Constant: '<Root>/sigma_bg_c'
  //   Constant: '<Root>/sigma_gyr_c'
  //   SignalConversion generated from: '<S4>/ SFunction '
  //   UnitDelay: '<Root>/Unit Delay'
  //   UnitDelay: '<Root>/Unit Delay1'

  if (*rtu_IMU_BUS_Valid) {
    localB->v[0] = sigma_acc;
    localB->v[3] = sigma_gyr;
    localB->v[6] = sigma_ba;
    localB->v[9] = sigma_bg;
    localB->v[1] = sigma_acc;
    localB->v[4] = sigma_gyr;
    localB->v[7] = sigma_ba;
    localB->v[10] = sigma_bg;
    localB->v[2] = sigma_acc;
    localB->v[5] = sigma_gyr;
    localB->v[8] = sigma_ba;
    localB->v[11] = sigma_bg;
    std::memset(&localB->c_b[0], 0, 144U * sizeof(real32_T));
    for (localB->j = 0; localB->j < 12; localB->j++) {
      localB->c_b[localB->j + 12 * localB->j] = localB->v[localB->j];
    }

    localB->accel_corr_idx_0 = rtu_IMU_BUS_Accel[0] - localDW->UnitDelay_DSTATE
      [10];
    localB->smax = rtu_IMU_BUS_Gyro[0] - localDW->UnitDelay_DSTATE[13];
    localB->omega_corr_idx_0 = localB->smax;
    localB->delta_idx_0 = localB->smax * ekf_predict_dt;
    localB->accel_corr_idx_1 = rtu_IMU_BUS_Accel[1] - localDW->UnitDelay_DSTATE
      [11];
    localB->smax = rtu_IMU_BUS_Gyro[1] - localDW->UnitDelay_DSTATE[14];
    localB->omega_corr_idx_1 = localB->smax;
    localB->delta_idx_1 = localB->smax * ekf_predict_dt;
    localB->accel_corr_idx_2 = rtu_IMU_BUS_Accel[2] - localDW->UnitDelay_DSTATE
      [12];
    localB->smax = rtu_IMU_BUS_Gyro[2] - localDW->UnitDelay_DSTATE[15];
    localB->delta_idx_2 = localB->smax * ekf_predict_dt;
    localB->theta = std::sqrt((localB->delta_idx_0 * localB->delta_idx_0 +
      localB->delta_idx_1 * localB->delta_idx_1) + localB->delta_idx_2 *
      localB->delta_idx_2);
    localB->rtb_TmpSignalConversionAtSFun_g = 0.0F;
    localB->rtb_TmpSignalConversionAtSFun_m = 0.0F;
    localB->rtb_TmpSignalConversionAtSFun_n = 0.0F;
    if (localB->theta > 1.0E-12) {
      localB->rtb_TmpSignalConversionAtSFun_n = 1.0F / localB->theta;
      localB->rtb_TmpSignalConversionAtSFun_g = localB->theta / 2.0F;
      localB->theta = std::cos(localB->rtb_TmpSignalConversionAtSFun_g);
      localB->e_b = std::sin(localB->rtb_TmpSignalConversionAtSFun_g);
      localB->rtb_TmpSignalConversionAtSFun_g = localB->delta_idx_0 *
        localB->rtb_TmpSignalConversionAtSFun_n * localB->e_b;
      localB->rtb_TmpSignalConversionAtSFun_m = localB->delta_idx_1 *
        localB->rtb_TmpSignalConversionAtSFun_n * localB->e_b;
      localB->rtb_TmpSignalConversionAtSFun_n = localB->delta_idx_2 *
        localB->rtb_TmpSignalConversionAtSFun_n * localB->e_b;
    } else {
      localB->theta = 1.0F;
    }

    _mm_storeu_ps(&localB->fv2[0], _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps
      (_mm_set_ps(localB->rtb_TmpSignalConversionAtSFun_n,
                  localB->rtb_TmpSignalConversionAtSFun_m,
                  localB->rtb_TmpSignalConversionAtSFun_g, localB->theta),
       _mm_set1_ps(localDW->UnitDelay_DSTATE[6])), _mm_mul_ps(_mm_mul_ps
      (_mm_set_ps(localB->rtb_TmpSignalConversionAtSFun_m,
                  localB->rtb_TmpSignalConversionAtSFun_n, localB->theta,
                  localB->rtb_TmpSignalConversionAtSFun_g), _mm_set1_ps
       (localDW->UnitDelay_DSTATE[7])), _mm_set_ps(1.0F, -1.0F, 1.0F, -1.0F))),
      _mm_mul_ps(_mm_mul_ps(_mm_set_ps(localB->rtb_TmpSignalConversionAtSFun_g,
      localB->theta, localB->rtb_TmpSignalConversionAtSFun_n,
      localB->rtb_TmpSignalConversionAtSFun_m), _mm_set1_ps
      (localDW->UnitDelay_DSTATE[8])), _mm_set_ps(-1.0F, 1.0F, 1.0F, -1.0F))),
      _mm_mul_ps(_mm_mul_ps(_mm_set_ps(localB->theta,
      localB->rtb_TmpSignalConversionAtSFun_g,
      localB->rtb_TmpSignalConversionAtSFun_m,
      localB->rtb_TmpSignalConversionAtSFun_n), _mm_set1_ps
      (localDW->UnitDelay_DSTATE[9])), _mm_set_ps(1.0F, 1.0F, -1.0F, -1.0F))));
    localB->theta = std::sqrt(((localB->fv2[0] * localB->fv2[0] + localB->fv2[1]
      * localB->fv2[1]) + localB->fv2[2] * localB->fv2[2]) + localB->fv2[3] *
      localB->fv2[3]);
    localB->delta_idx_0 = localDW->UnitDelay_DSTATE[9] *
      localDW->UnitDelay_DSTATE[9];
    localB->delta_idx_1 = localDW->UnitDelay_DSTATE[8] *
      localDW->UnitDelay_DSTATE[8];
    localB->C_bn[0] = 1.0F - (localB->delta_idx_1 + localB->delta_idx_0) * 2.0F;
    localB->delta_idx_2 = localDW->UnitDelay_DSTATE[7] *
      localDW->UnitDelay_DSTATE[8];
    localB->rtb_TmpSignalConversionAtSFun_g = localDW->UnitDelay_DSTATE[6] *
      localDW->UnitDelay_DSTATE[9];
    localB->C_bn[3] = (localB->delta_idx_2 -
                       localB->rtb_TmpSignalConversionAtSFun_g) * 2.0F;
    localB->rtb_TmpSignalConversionAtSFun_m = localDW->UnitDelay_DSTATE[7] *
      localDW->UnitDelay_DSTATE[9];
    localB->rtb_TmpSignalConversionAtSFun_n = localDW->UnitDelay_DSTATE[6] *
      localDW->UnitDelay_DSTATE[8];
    localB->C_bn[6] = (localB->rtb_TmpSignalConversionAtSFun_m +
                       localB->rtb_TmpSignalConversionAtSFun_n) * 2.0F;
    localB->C_bn[1] = (localB->delta_idx_2 +
                       localB->rtb_TmpSignalConversionAtSFun_g) * 2.0F;
    localB->delta_idx_2 = localDW->UnitDelay_DSTATE[7] *
      localDW->UnitDelay_DSTATE[7];
    localB->C_bn[4] = 1.0F - (localB->delta_idx_2 + localB->delta_idx_0) * 2.0F;
    localB->delta_idx_0 = localDW->UnitDelay_DSTATE[8] *
      localDW->UnitDelay_DSTATE[9];
    localB->rtb_TmpSignalConversionAtSFun_g = localDW->UnitDelay_DSTATE[6] *
      localDW->UnitDelay_DSTATE[7];
    localB->C_bn[7] = (localB->delta_idx_0 -
                       localB->rtb_TmpSignalConversionAtSFun_g) * 2.0F;
    localB->C_bn[2] = (localB->rtb_TmpSignalConversionAtSFun_m -
                       localB->rtb_TmpSignalConversionAtSFun_n) * 2.0F;
    localB->C_bn[5] = (localB->delta_idx_0 +
                       localB->rtb_TmpSignalConversionAtSFun_g) * 2.0F;
    localB->C_bn[8] = 1.0F - (localB->delta_idx_2 + localB->delta_idx_1) * 2.0F;
    localB->delta_idx_0 = ekf_predict_dt * ekf_predict_dt;
    for (localB->j = 0; localB->j < 3; localB->j++) {
      localB->delta_idx_1 = ((localB->C_bn[localB->j + 3] *
        localB->accel_corr_idx_1 + localB->C_bn[localB->j] *
        localB->accel_corr_idx_0) + localB->C_bn[localB->j + 6] *
        localB->accel_corr_idx_2) + g_n[localB->j];
      localB->delta_idx_2 = localDW->UnitDelay_DSTATE[localB->j + 3];
      localB->fv[localB->j] = (localB->delta_idx_2 * ekf_predict_dt +
        localDW->UnitDelay_DSTATE[localB->j]) + 0.5F * localB->delta_idx_1 *
        localB->delta_idx_0;
      localB->fv[localB->j + 3] = localB->delta_idx_1 * ekf_predict_dt +
        localB->delta_idx_2;
    }

    localB->fv[6] = localB->fv2[0] / localB->theta;
    localB->fv[7] = localB->fv2[1] / localB->theta;
    localB->fv[8] = localB->fv2[2] / localB->theta;
    localB->fv[9] = localB->fv2[3] / localB->theta;
    localB->fv[10] = localDW->UnitDelay_DSTATE[10];
    localB->fv[13] = localDW->UnitDelay_DSTATE[13];
    localB->fv[11] = localDW->UnitDelay_DSTATE[11];
    localB->fv[14] = localDW->UnitDelay_DSTATE[14];
    localB->fv[12] = localDW->UnitDelay_DSTATE[12];
    localB->fv[15] = localDW->UnitDelay_DSTATE[15];
    std::memcpy(&localDW->UnitDelay_DSTATE[0], &localB->fv[0], sizeof(real32_T) <<
                4U);
    std::memset(&localB->a[0], 0, 225U * sizeof(real_T));
    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->b_b[localB->j] = 0.0F;
    }

    localB->b_b[3] = -localB->accel_corr_idx_2;
    localB->b_b[6] = localB->accel_corr_idx_1;
    localB->b_b[1] = localB->accel_corr_idx_2;
    localB->b_b[7] = -localB->accel_corr_idx_0;
    localB->b_b[2] = -localB->accel_corr_idx_1;
    localB->b_b[5] = localB->accel_corr_idx_0;
    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->w_sk[localB->j] = 0.0F;
    }

    localB->w_sk[3] = -localB->smax;
    localB->w_sk[6] = localB->omega_corr_idx_1;
    localB->w_sk[1] = localB->smax;
    localB->w_sk[7] = -localB->omega_corr_idx_0;
    localB->w_sk[2] = -localB->omega_corr_idx_1;
    localB->w_sk[5] = localB->omega_corr_idx_0;
    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->b_I[localB->j] = 0;
    }

    localB->b_I[0] = 1;
    localB->b_I[4] = 1;
    localB->b_I[8] = 1;
    for (localB->j = 0; localB->j < 3; localB->j++) {
      a_tmp = (localB->j + 3) * 15;
      localB->a[a_tmp] = localB->b_I[3 * localB->j];
      localB->a[a_tmp + 1] = localB->b_I[3 * localB->j + 1];
      localB->a[a_tmp + 2] = localB->b_I[3 * localB->j + 2];
    }

    for (localB->j = 0; localB->j <= 4; localB->j += 4) {
      tmp_0 = _mm_loadu_ps(&localB->C_bn[localB->j]);
      _mm_storeu_ps(&localB->C_bn_p[localB->j], _mm_mul_ps(tmp_0, _mm_set1_ps
        (-1.0F)));
    }

    for (localB->j = 8; localB->j < 9; localB->j++) {
      localB->C_bn_p[localB->j] = -localB->C_bn[localB->j];
    }

    for (localB->j = 0; localB->j < 3; localB->j++) {
      localB->accel_corr_idx_0 = localB->C_bn_p[localB->j + 3];
      localB->accel_corr_idx_1 = localB->C_bn_p[localB->j];
      localB->accel_corr_idx_2 = localB->C_bn_p[localB->j + 6];
      for (a_tmp = 0; a_tmp < 3; a_tmp++) {
        localB->a[(localB->j + 15 * (a_tmp + 6)) + 3] = (localB->b_b[3 * a_tmp +
          1] * localB->accel_corr_idx_0 + localB->b_b[3 * a_tmp] *
          localB->accel_corr_idx_1) + localB->b_b[3 * a_tmp + 2] *
          localB->accel_corr_idx_2;
      }
    }

    for (localB->j = 0; localB->j < 3; localB->j++) {
      a_tmp = (localB->j + 9) * 15;
      localB->a[a_tmp + 3] = -localB->C_bn[3 * localB->j];
      a_tmp_0 = (localB->j + 6) * 15;
      localB->a[a_tmp_0 + 6] = -localB->w_sk[3 * localB->j];
      localB->jj = 3 * localB->j + 1;
      localB->a[a_tmp + 4] = -localB->C_bn[localB->jj];
      localB->a[a_tmp_0 + 7] = -localB->w_sk[localB->jj];
      localB->jj = 3 * localB->j + 2;
      localB->a[a_tmp + 5] = -localB->C_bn[localB->jj];
      localB->a[a_tmp_0 + 8] = -localB->w_sk[localB->jj];
    }

    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->b_I[localB->j] = 0;
    }

    localB->b_I[0] = 1;
    localB->b_I[4] = 1;
    localB->b_I[8] = 1;
    for (localB->j = 0; localB->j < 3; localB->j++) {
      a_tmp = (localB->j + 12) * 15;
      localB->a[a_tmp + 6] = -static_cast<real_T>(localB->b_I[3 * localB->j]);
      localB->a[a_tmp + 7] = -static_cast<real_T>(localB->b_I[3 * localB->j + 1]);
      localB->a[a_tmp + 8] = -static_cast<real_T>(localB->b_I[3 * localB->j + 2]);
    }

    std::memset(&localB->c_I[0], 0, 225U * sizeof(int8_T));
    for (localB->j = 0; localB->j < 15; localB->j++) {
      localB->c_I[localB->j + 15 * localB->j] = 1;
    }

    for (localB->j = 0; localB->j <= 220; localB->j += 4) {
      _mm_storeu_ps(&localB->F[localB->j], _mm_add_ps(_mm_mul_ps(_mm_set_ps(
        static_cast<real32_T>(localB->a[localB->j + 3]), static_cast<real32_T>
        (localB->a[localB->j + 2]), static_cast<real32_T>(localB->a[localB->j +
        1]), static_cast<real32_T>(localB->a[localB->j])), _mm_set1_ps
        (ekf_predict_dt)), _mm_set_ps(static_cast<real32_T>(localB->c_I
        [localB->j + 3]), static_cast<real32_T>(localB->c_I[localB->j + 2]),
        static_cast<real32_T>(localB->c_I[localB->j + 1]), static_cast<real32_T>
        (localB->c_I[localB->j]))));
    }

    for (localB->j = 224; localB->j < 225; localB->j++) {
      localB->F[localB->j] = static_cast<real32_T>(localB->a[localB->j]) *
        ekf_predict_dt + static_cast<real32_T>(localB->c_I[localB->j]);
    }

    std::memset(&localB->G[0], 0, 180U * sizeof(real_T));
    for (localB->j = 0; localB->j < 3; localB->j++) {
      localB->G[15 * localB->j + 3] = -localB->C_bn[3 * localB->j];
      localB->G[15 * localB->j + 4] = -localB->C_bn[3 * localB->j + 1];
      localB->G[15 * localB->j + 5] = -localB->C_bn[3 * localB->j + 2];
    }

    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->b_I[localB->j] = 0;
    }

    localB->b_I[0] = 1;
    localB->b_I[4] = 1;
    localB->b_I[8] = 1;
    for (localB->j = 0; localB->j < 3; localB->j++) {
      localB->jj = (localB->j + 3) * 15;
      localB->G[localB->jj + 6] = -static_cast<real_T>(localB->b_I[3 * localB->j]);
      localB->G[localB->jj + 7] = -static_cast<real_T>(localB->b_I[3 * localB->j
        + 1]);
      localB->G[localB->jj + 8] = -static_cast<real_T>(localB->b_I[3 * localB->j
        + 2]);
    }

    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->b_I[localB->j] = 0;
    }

    localB->b_I[0] = 1;
    localB->b_I[4] = 1;
    localB->b_I[8] = 1;
    for (localB->j = 0; localB->j < 3; localB->j++) {
      localB->jj = (localB->j + 6) * 15;
      localB->G[localB->jj + 9] = localB->b_I[3 * localB->j];
      localB->G[localB->jj + 10] = localB->b_I[3 * localB->j + 1];
      localB->G[localB->jj + 11] = localB->b_I[3 * localB->j + 2];
    }

    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->b_I[localB->j] = 0;
    }

    localB->b_I[0] = 1;
    localB->b_I[4] = 1;
    localB->b_I[8] = 1;
    for (localB->j = 0; localB->j < 3; localB->j++) {
      localB->jj = (localB->j + 9) * 15;
      localB->G[localB->jj + 12] = localB->b_I[3 * localB->j];
      localB->G[localB->jj + 13] = localB->b_I[3 * localB->j + 1];
      localB->G[localB->jj + 14] = localB->b_I[3 * localB->j + 2];
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->F_c[a_tmp + 15 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->delta_idx_1 = localDW->UnitDelay1_DSTATE[15 * localB->j + a_tmp];
        for (a_tmp_0 = 0; a_tmp_0 <= 8; a_tmp_0 += 4) {
          tmp_0 = _mm_loadu_ps(&localB->F[15 * a_tmp + a_tmp_0]);
          localB->jj = 15 * localB->j + a_tmp_0;
          tmp = _mm_loadu_ps(&localB->F_c[localB->jj]);
          _mm_storeu_ps(&localB->F_c[localB->jj], _mm_add_ps(_mm_mul_ps(tmp_0,
            _mm_set1_ps(localB->delta_idx_1)), tmp));
        }

        for (a_tmp_0 = 12; a_tmp_0 < 15; a_tmp_0++) {
          localB->jj = 15 * localB->j + a_tmp_0;
          localB->F_c[localB->jj] += localB->F[15 * a_tmp + a_tmp_0] *
            localB->delta_idx_1;
        }
      }
    }

    for (localB->j = 0; localB->j < 12; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->G_c[a_tmp + 15 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 12; a_tmp++) {
        localB->delta_idx_1 = localB->c_b[12 * localB->j + a_tmp];
        for (a_tmp_0 = 0; a_tmp_0 < 15; a_tmp_0++) {
          localB->jj = 15 * localB->j + a_tmp_0;
          localB->G_c[localB->jj] += static_cast<real32_T>(localB->G[15 * a_tmp
            + a_tmp_0]) * localB->delta_idx_1;
        }
      }
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->G_k[a_tmp + 15 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 12; a_tmp++) {
        localB->delta_idx_1 = static_cast<real32_T>(localB->G[15 * a_tmp +
          localB->j]);
        for (a_tmp_0 = 0; a_tmp_0 <= 8; a_tmp_0 += 4) {
          tmp_0 = _mm_loadu_ps(&localB->G_c[15 * a_tmp + a_tmp_0]);
          localB->jj = 15 * localB->j + a_tmp_0;
          tmp = _mm_loadu_ps(&localB->G_k[localB->jj]);
          _mm_storeu_ps(&localB->G_k[localB->jj], _mm_add_ps(_mm_mul_ps(tmp_0,
            _mm_set1_ps(localB->delta_idx_1)), tmp));
        }

        for (a_tmp_0 = 12; a_tmp_0 < 15; a_tmp_0++) {
          localB->jj = 15 * localB->j + a_tmp_0;
          localB->G_k[localB->jj] += localB->G_c[15 * a_tmp + a_tmp_0] *
            localB->delta_idx_1;
        }
      }
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->delta_idx_1 = 0.0F;
        for (a_tmp_0 = 0; a_tmp_0 < 15; a_tmp_0++) {
          localB->delta_idx_1 += localB->F_c[15 * a_tmp_0 + localB->j] *
            localB->F[15 * a_tmp_0 + a_tmp];
        }

        P_p_tmp = 15 * a_tmp + localB->j;
        localB->P_p[P_p_tmp] = localB->G_k[P_p_tmp] * ekf_predict_dt +
          localB->delta_idx_1;
      }
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        a_tmp_0 = 15 * localB->j + a_tmp;
        localDW->UnitDelay1_DSTATE[a_tmp_0] = (localB->P_p[15 * a_tmp +
          localB->j] + localB->P_p[a_tmp_0]) / 2.0F;
      }
    }
  }

  // End of MATLAB Function: '<Root>/MATLAB Function'

  // Logic: '<Root>/GpsUpdateEnable'
  rtb_GpsUpdateEnable = ((*rtu_GPS_BUS_Valid) && (*rtu_GPS_BUS_IsNew));

  // MATLAB Function: '<S3>/FirstFixOriginLatch'
  if (rtb_GpsUpdateEnable && (!localDW->initialized)) {
    localDW->stored_origin[0] = *rtu_GPS_BUS_Lat;
    localDW->stored_origin[1] = *rtu_GPS_BUS_Lon;
    localDW->stored_origin[2] = *rtu_GPS_BUS_Alt;
    localDW->initialized = true;
  }

  // MATLAB Function: '<S3>/MATLAB Function' incorporates:
  //   DataTypeConversion: '<S3>/AltToDouble'
  //   DataTypeConversion: '<S3>/LatToDouble'
  //   DataTypeConversion: '<S3>/LonToDouble'
  //   MATLAB Function: '<S3>/FirstFixOriginLatch'

  localB->accel_corr_idx_0 = 0.0F;
  localB->accel_corr_idx_1 = 0.0F;
  localB->accel_corr_idx_2 = 0.0F;
  if (localDW->initialized) {
    localB->cosphi = localDW->stored_origin[0];
    EKF_cosd_ZK11uGDT(&localB->cosphi, localB);
    localB->sinphi = localDW->stored_origin[0];
    EKF_sind_CjVy6Jw4(&localB->sinphi, localB);
    localB->coslambda = localDW->stored_origin[1];
    EKF_cosd_ZK11uGDT(&localB->coslambda, localB);
    localB->sinlambda = localDW->stored_origin[1];
    EKF_sind_CjVy6Jw4(&localB->sinlambda, localB);
    localB->rtu_GPS_BUS_Lat[0] = *rtu_GPS_BUS_Lat;
    localB->rtu_GPS_BUS_Lat[1] = *rtu_GPS_BUS_Lon;
    localB->rtu_GPS_BUS_Lat[2] = *rtu_GPS_BUS_Alt;
    EKF_lla2ecef_WvPbxdnX(localB->rtu_GPS_BUS_Lat, localB->dv, localB);
    EKF_lla2ecef_WvPbxdnX(localDW->stored_origin, localB->rtu_GPS_BUS_Lat,
                          localB);
    tmp_1 = _mm_sub_pd(_mm_loadu_pd(&localB->dv[0]), _mm_loadu_pd
                       (&localB->rtu_GPS_BUS_Lat[0]));
    _mm_storeu_pd(&localB->dv1[0], tmp_1);
    localB->ecefPosWithENUOrigin_idx_2 = localB->dv[2] - localB->
      rtu_GPS_BUS_Lat[2];
    localB->tmp = localB->coslambda * localB->dv1[0] + localB->sinlambda *
      localB->dv1[1];
    localB->accel_corr_idx_0 = static_cast<real32_T>(-localB->sinphi *
      localB->tmp + localB->cosphi * localB->ecefPosWithENUOrigin_idx_2);
    localB->accel_corr_idx_1 = static_cast<real32_T>(-localB->sinlambda *
      localB->dv1[0] + localB->coslambda * localB->dv1[1]);
    localB->accel_corr_idx_2 = static_cast<real32_T>(-(localB->cosphi *
      localB->tmp + localB->sinphi * localB->ecefPosWithENUOrigin_idx_2));
  }

  // End of MATLAB Function: '<S3>/MATLAB Function'

  // Outputs for Enabled SubSystem: '<Root>/EKFUpdate' incorporates:
  //   EnablePort: '<S2>/Enable'

  // Switch: '<Root>/Switch1' incorporates:
  //   Logic: '<S3>/Logical Operator'
  //   MATLAB Function: '<S2>/EKF_Update'
  //   MATLAB Function: '<S3>/FirstFixOriginLatch'
  //   Switch: '<Root>/Switch'
  //   UnitDelay: '<Root>/Unit Delay'
  //   UnitDelay: '<Root>/Unit Delay1'

  if (localDW->initialized && rtb_GpsUpdateEnable) {
    // MATLAB Function: '<S2>/EKF_Update' incorporates:
    //   Constant: '<S2>/R_pos'
    //   Constant: '<S2>/R_vel'
    //   SignalConversion generated from: '<S6>/ SFunction '

    std::memset(&localB->H[0], 0, 90U * sizeof(int8_T));
    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->b_I[localB->j] = 0;
    }

    localB->b_I[0] = 1;
    localB->b_I[4] = 1;
    localB->b_I[8] = 1;
    for (localB->j = 0; localB->j < 3; localB->j++) {
      localB->H[6 * localB->j] = localB->b_I[3 * localB->j];
      localB->H[6 * localB->j + 1] = localB->b_I[3 * localB->j + 1];
      localB->H[6 * localB->j + 2] = localB->b_I[3 * localB->j + 2];
    }

    for (localB->j = 0; localB->j < 9; localB->j++) {
      localB->b_I[localB->j] = 0;
    }

    localB->b_I[0] = 1;
    localB->b_I[4] = 1;
    localB->b_I[8] = 1;
    for (localB->j = 0; localB->j < 3; localB->j++) {
      a_tmp = (localB->j + 3) * 6;
      localB->H[a_tmp + 3] = localB->b_I[3 * localB->j];
      localB->H[a_tmp + 4] = localB->b_I[3 * localB->j + 1];
      localB->H[a_tmp + 5] = localB->b_I[3 * localB->j + 2];
      localB->inertial[localB->j] = R_pos;
      localB->inertial[localB->j + 3] = R_vel;
    }

    std::memset(&localB->R[0], 0, 36U * sizeof(real32_T));
    for (localB->j = 0; localB->j < 6; localB->j++) {
      localB->R[localB->j + 6 * localB->j] = localB->inertial[localB->j];
    }

    std::memcpy(&localB->A_tmp_b[0], &localB->H[0], 90U * sizeof(int8_T));
    for (localB->j = 0; localB->j < 6; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->H[a_tmp + 15 * localB->j] = localB->A_tmp_b[6 * a_tmp +
          localB->j];
      }
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 6; a_tmp++) {
        localB->A_tmp[a_tmp + 6 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->delta_idx_1 = localDW->UnitDelay1_DSTATE[15 * localB->j + a_tmp];
        for (a_tmp_0 = 0; a_tmp_0 < 6; a_tmp_0++) {
          localB->jj = 6 * localB->j + a_tmp_0;
          localB->A_tmp[localB->jj] += static_cast<real32_T>(localB->A_tmp_b[6 *
            a_tmp + a_tmp_0]) * localB->delta_idx_1;
        }
      }
    }

    for (localB->j = 0; localB->j < 6; localB->j++) {
      for (a_tmp = 0; a_tmp < 6; a_tmp++) {
        localB->delta_idx_1 = 0.0F;
        for (a_tmp_0 = 0; a_tmp_0 < 15; a_tmp_0++) {
          localB->delta_idx_1 += localB->A_tmp[6 * a_tmp_0 + localB->j] *
            static_cast<real32_T>(localB->H[15 * a_tmp + a_tmp_0]);
        }

        a_tmp_0 = 6 * a_tmp + localB->j;
        localB->A[a_tmp_0] = localB->R[a_tmp_0] + localB->delta_idx_1;
      }

      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->X[a_tmp + 15 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        a_tmp_0 = localB->H[15 * localB->j + a_tmp];
        for (localB->jj = 0; localB->jj <= 8; localB->jj += 4) {
          tmp_0 = _mm_loadu_ps(&localDW->UnitDelay1_DSTATE[15 * a_tmp +
                               localB->jj]);
          P_p_tmp = 15 * localB->j + localB->jj;
          tmp = _mm_loadu_ps(&localB->X[P_p_tmp]);
          _mm_storeu_ps(&localB->X[P_p_tmp], _mm_add_ps(_mm_mul_ps(tmp_0,
            _mm_set1_ps(static_cast<real32_T>(a_tmp_0))), tmp));
        }

        for (localB->jj = 12; localB->jj < 15; localB->jj++) {
          P_p_tmp = 15 * localB->j + localB->jj;
          localB->X[P_p_tmp] += localDW->UnitDelay1_DSTATE[15 * a_tmp +
            localB->jj] * static_cast<real32_T>(a_tmp_0);
        }
      }

      localB->ipiv[localB->j] = static_cast<int8_T>(localB->j + 1);
    }

    for (localB->j = 0; localB->j < 5; localB->j++) {
      localB->jj = localB->j * 7;
      a_tmp_0 = 7 - localB->j;
      P_p_tmp = 0;
      localB->smax = std::abs(localB->A[localB->jj]);
      for (a_tmp = 2; a_tmp < a_tmp_0; a_tmp++) {
        localB->omega_corr_idx_0 = std::abs(localB->A[(localB->jj + a_tmp) - 1]);
        if (localB->omega_corr_idx_0 > localB->smax) {
          P_p_tmp = a_tmp - 1;
          localB->smax = localB->omega_corr_idx_0;
        }
      }

      if (localB->A[localB->jj + P_p_tmp] != 0.0F) {
        if (P_p_tmp != 0) {
          P_p_tmp += localB->j;
          localB->ipiv[localB->j] = static_cast<int8_T>(P_p_tmp + 1);
          for (a_tmp = 0; a_tmp < 6; a_tmp++) {
            localB->jA = a_tmp * 6 + localB->j;
            localB->smax = localB->A[localB->jA];
            a_tmp_0 = a_tmp * 6 + P_p_tmp;
            localB->A[localB->jA] = localB->A[a_tmp_0];
            localB->A[a_tmp_0] = localB->smax;
          }
        }

        a_tmp_0 = (localB->jj - localB->j) + 6;
        P_p_tmp = (((((a_tmp_0 - localB->jj) - 1) / 4) << 2) + localB->jj) + 2;
        localB->jA = P_p_tmp - 4;
        for (a_tmp = localB->jj + 2; a_tmp <= localB->jA; a_tmp += 4) {
          tmp_0 = _mm_loadu_ps(&localB->A[a_tmp - 1]);
          _mm_storeu_ps(&localB->A[a_tmp - 1], _mm_div_ps(tmp_0, _mm_set1_ps
            (localB->A[localB->jj])));
        }

        for (a_tmp = P_p_tmp; a_tmp <= a_tmp_0; a_tmp++) {
          localB->A[a_tmp - 1] /= localB->A[localB->jj];
        }
      }

      P_p_tmp = 4 - localB->j;
      localB->jA = localB->jj + 8;
      for (a_tmp = 0; a_tmp <= P_p_tmp; a_tmp++) {
        localB->delta_idx_1 = localB->A[(a_tmp * 6 + localB->jj) + 6];
        if (localB->delta_idx_1 != 0.0F) {
          localB->kBcol = (localB->jA - localB->j) + 4;
          for (a_tmp_0 = localB->jA; a_tmp_0 <= localB->kBcol; a_tmp_0++) {
            localB->A[a_tmp_0 - 1] += localB->A[((localB->jj + a_tmp_0) -
              localB->jA) + 1] * -localB->delta_idx_1;
          }
        }

        localB->jA += 6;
      }
    }

    for (a_tmp = 0; a_tmp < 6; a_tmp++) {
      localB->jj = 15 * a_tmp;
      localB->jA = 6 * a_tmp;
      for (localB->j = 0; localB->j < a_tmp; localB->j++) {
        localB->kBcol = 15 * localB->j;
        localB->delta_idx_1 = localB->A[localB->j + localB->jA];
        if (localB->delta_idx_1 != 0.0F) {
          for (a_tmp_0 = 0; a_tmp_0 < 15; a_tmp_0++) {
            P_p_tmp = a_tmp_0 + localB->jj;
            localB->X[P_p_tmp] -= localB->X[a_tmp_0 + localB->kBcol] *
              localB->delta_idx_1;
          }
        }
      }

      localB->smax = 1.0F / localB->A[a_tmp + localB->jA];
      for (a_tmp_0 = 0; a_tmp_0 <= 8; a_tmp_0 += 4) {
        localB->j = a_tmp_0 + localB->jj;
        tmp_0 = _mm_loadu_ps(&localB->X[localB->j]);
        _mm_storeu_ps(&localB->X[localB->j], _mm_mul_ps(tmp_0, _mm_set1_ps
          (localB->smax)));
      }

      for (a_tmp_0 = 12; a_tmp_0 < 15; a_tmp_0++) {
        P_p_tmp = a_tmp_0 + localB->jj;
        localB->X[P_p_tmp] *= localB->smax;
      }
    }

    for (localB->j = 5; localB->j >= 0; localB->j--) {
      localB->jj = 15 * localB->j;
      localB->jA = 6 * localB->j - 1;
      for (a_tmp = localB->j + 2; a_tmp < 7; a_tmp++) {
        localB->kBcol = (a_tmp - 1) * 15;
        localB->delta_idx_1 = localB->A[a_tmp + localB->jA];
        if (localB->delta_idx_1 != 0.0F) {
          for (a_tmp_0 = 0; a_tmp_0 < 15; a_tmp_0++) {
            P_p_tmp = a_tmp_0 + localB->jj;
            localB->X[P_p_tmp] -= localB->X[a_tmp_0 + localB->kBcol] *
              localB->delta_idx_1;
          }
        }
      }
    }

    for (a_tmp = 4; a_tmp >= 0; a_tmp--) {
      ipiv = localB->ipiv[a_tmp];
      if (a_tmp + 1 != ipiv) {
        for (localB->j = 0; localB->j < 15; localB->j++) {
          a_tmp_0 = 15 * a_tmp + localB->j;
          localB->smax = localB->X[a_tmp_0];
          P_p_tmp = (ipiv - 1) * 15 + localB->j;
          localB->X[a_tmp_0] = localB->X[P_p_tmp];
          localB->X[P_p_tmp] = localB->smax;
        }
      }
    }

    localB->inertial[0] = localB->accel_corr_idx_0;
    localB->inertial[3] = rtu_GPS_BUS_Velocity[0];
    localB->fv1[0] = localDW->UnitDelay_DSTATE[0];
    localB->fv1[3] = localDW->UnitDelay_DSTATE[3];
    localB->inertial[1] = localB->accel_corr_idx_1;
    localB->inertial[4] = rtu_GPS_BUS_Velocity[1];
    localB->fv1[1] = localDW->UnitDelay_DSTATE[1];
    localB->fv1[4] = localDW->UnitDelay_DSTATE[4];
    localB->inertial[2] = localB->accel_corr_idx_2;
    localB->inertial[5] = rtu_GPS_BUS_Velocity[2];
    localB->fv1[2] = localDW->UnitDelay_DSTATE[2];
    localB->fv1[5] = localDW->UnitDelay_DSTATE[5];
    for (localB->j = 0; localB->j <= 0; localB->j += 4) {
      tmp_0 = _mm_loadu_ps(&localB->inertial[localB->j]);
      tmp = _mm_loadu_ps(&localB->fv1[localB->j]);
      _mm_storeu_ps(&localB->accel_corr[localB->j], _mm_sub_ps(tmp_0, tmp));
    }

    for (localB->j = 4; localB->j < 6; localB->j++) {
      localB->accel_corr[localB->j] = localB->inertial[localB->j] - localB->
        fv1[localB->j];
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      localB->delta_x[localB->j] = 0.0F;
    }

    for (localB->j = 0; localB->j < 6; localB->j++) {
      localB->delta_idx_1 = localB->accel_corr[localB->j];
      for (a_tmp = 0; a_tmp <= 8; a_tmp += 4) {
        tmp_0 = _mm_loadu_ps(&localB->X[15 * localB->j + a_tmp]);
        tmp = _mm_loadu_ps(&localB->delta_x[a_tmp]);
        _mm_storeu_ps(&localB->delta_x[a_tmp], _mm_add_ps(_mm_mul_ps(tmp_0,
          _mm_set1_ps(localB->delta_idx_1)), tmp));
      }

      for (a_tmp = 12; a_tmp < 15; a_tmp++) {
        localB->delta_x[a_tmp] += localB->X[15 * localB->j + a_tmp] *
          localB->delta_idx_1;
      }
    }

    std::memcpy(&localB->x_c[0], &localDW->UnitDelay_DSTATE[0], sizeof(real32_T)
                << 4U);
    _mm_storeu_ps(&localB->fv2[0], _mm_add_ps(_mm_set_ps
      (localDW->UnitDelay_DSTATE[4], localDW->UnitDelay_DSTATE[1],
       localDW->UnitDelay_DSTATE[3], localDW->UnitDelay_DSTATE[0]), _mm_set_ps
      (localB->delta_x[4], localB->delta_x[1], localB->delta_x[3],
       localB->delta_x[0])));
    localB->x_c[0] = localB->fv2[0];
    localB->x_c[3] = localB->fv2[1];
    localB->x_c[1] = localB->fv2[2];
    localB->x_c[4] = localB->fv2[3];

    // MATLAB Function: '<S2>/EKF_Update'
    localB->x_c[2] = localDW->UnitDelay_DSTATE[2] + localB->delta_x[2];
    localB->x_c[5] = localDW->UnitDelay_DSTATE[5] + localB->delta_x[5];
    localB->theta = std::sqrt((localB->delta_x[6] * localB->delta_x[6] +
      localB->delta_x[7] * localB->delta_x[7]) + localB->delta_x[8] *
      localB->delta_x[8]);
    localB->accel_corr_idx_0 = 0.0F;
    localB->accel_corr_idx_1 = 0.0F;
    localB->smax = 0.0F;
    if (localB->theta > 1.0E-12) {
      localB->delta_idx_0 = 1.0F / localB->theta;
      localB->accel_corr_idx_0 = localB->theta / 2.0F;
      localB->accel_corr_idx_2 = std::cos(localB->accel_corr_idx_0);
      localB->smax = std::sin(localB->accel_corr_idx_0);
      localB->accel_corr_idx_0 = localB->delta_x[6] * localB->delta_idx_0 *
        localB->smax;
      localB->accel_corr_idx_1 = localB->delta_x[7] * localB->delta_idx_0 *
        localB->smax;
      localB->smax *= localB->delta_x[8] * localB->delta_idx_0;
    } else {
      localB->accel_corr_idx_2 = 1.0F;
    }

    tmp_0 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps
      (localB->accel_corr_idx_2), _mm_loadu_ps(&localDW->UnitDelay_DSTATE[6])),
      _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(localB->accel_corr_idx_0), _mm_set_ps
      (localDW->UnitDelay_DSTATE[8], localDW->UnitDelay_DSTATE[9],
       localDW->UnitDelay_DSTATE[6], localDW->UnitDelay_DSTATE[7])), _mm_set_ps
                 (1.0F, -1.0F, 1.0F, -1.0F))), _mm_mul_ps(_mm_mul_ps(_mm_set1_ps
      (localB->accel_corr_idx_1), _mm_set_ps(localDW->UnitDelay_DSTATE[7],
      localDW->UnitDelay_DSTATE[6], localDW->UnitDelay_DSTATE[9],
      localDW->UnitDelay_DSTATE[8])), _mm_set_ps(-1.0F, 1.0F, 1.0F, -1.0F))),
                       _mm_mul_ps(_mm_mul_ps(_mm_set1_ps(localB->smax),
      _mm_set_ps(localDW->UnitDelay_DSTATE[6], localDW->UnitDelay_DSTATE[7],
                 localDW->UnitDelay_DSTATE[8], localDW->UnitDelay_DSTATE[9])),
      _mm_set_ps(1.0F, 1.0F, -1.0F, -1.0F)));
    _mm_storeu_ps(&localB->x_c[6], tmp_0);
    tmp_0 = _mm_div_ps(_mm_loadu_ps(&localB->x_c[6]), _mm_set1_ps(std::sqrt
      (((localB->x_c[6] * localB->x_c[6] + localB->x_c[7] * localB->x_c[7]) +
        localB->x_c[8] * localB->x_c[8]) + localB->x_c[9] * localB->x_c[9])));
    _mm_storeu_ps(&localB->x_c[6], tmp_0);
    _mm_storeu_ps(&localB->fv2[0], _mm_add_ps(_mm_set_ps(localB->delta_x[13],
      localB->delta_x[10], localB->delta_x[12], localB->delta_x[9]), _mm_set_ps
      (localDW->UnitDelay_DSTATE[14], localDW->UnitDelay_DSTATE[11],
       localDW->UnitDelay_DSTATE[13], localDW->UnitDelay_DSTATE[10])));
    localB->x_c[10] = localB->fv2[0];
    localB->x_c[13] = localB->fv2[1];
    localB->x_c[11] = localB->fv2[2];
    localB->x_c[14] = localB->fv2[3];

    // MATLAB Function: '<S2>/EKF_Update'
    localB->x_c[12] = localB->delta_x[11] + localDW->UnitDelay_DSTATE[12];
    localB->x_c[15] = localB->delta_x[14] + localDW->UnitDelay_DSTATE[15];
    for (localB->j = 0; localB->j < 225; localB->j++) {
      localB->P_p[localB->j] = e[localB->j];
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->G_k[a_tmp + 15 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 6; a_tmp++) {
        a_tmp_0 = localB->A_tmp_b[6 * localB->j + a_tmp];
        for (localB->jj = 0; localB->jj <= 8; localB->jj += 4) {
          tmp_0 = _mm_loadu_ps(&localB->X[15 * a_tmp + localB->jj]);
          P_p_tmp = 15 * localB->j + localB->jj;
          tmp = _mm_loadu_ps(&localB->G_k[P_p_tmp]);
          _mm_storeu_ps(&localB->G_k[P_p_tmp], _mm_add_ps(_mm_mul_ps(tmp_0,
            _mm_set1_ps(static_cast<real32_T>(a_tmp_0))), tmp));
        }

        for (localB->jj = 12; localB->jj < 15; localB->jj++) {
          P_p_tmp = 15 * localB->j + localB->jj;
          localB->G_k[P_p_tmp] += localB->X[15 * a_tmp + localB->jj] *
            static_cast<real32_T>(a_tmp_0);
        }
      }
    }

    for (localB->j = 0; localB->j <= 220; localB->j += 4) {
      tmp_0 = _mm_loadu_ps(&localB->P_p[localB->j]);
      tmp = _mm_loadu_ps(&localB->G_k[localB->j]);
      _mm_storeu_ps(&localB->P_p_m[localB->j], _mm_sub_ps(tmp_0, tmp));
    }

    for (localB->j = 224; localB->j < 225; localB->j++) {
      localB->P_p_m[localB->j] = localB->P_p[localB->j] - localB->G_k[localB->j];
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->F[a_tmp + 15 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        a_tmp_0 = 15 * localB->j + a_tmp;
        localB->delta_idx_1 = localDW->UnitDelay1_DSTATE[a_tmp_0];
        for (localB->jj = 0; localB->jj <= 8; localB->jj += 4) {
          tmp_0 = _mm_loadu_ps(&localB->P_p_m[15 * a_tmp + localB->jj]);
          P_p_tmp = 15 * localB->j + localB->jj;
          tmp = _mm_loadu_ps(&localB->F[P_p_tmp]);
          _mm_storeu_ps(&localB->F[P_p_tmp], _mm_add_ps(_mm_mul_ps(tmp_0,
            _mm_set1_ps(localB->delta_idx_1)), tmp));
        }

        for (localB->jj = 12; localB->jj < 15; localB->jj++) {
          P_p_tmp = 15 * localB->j + localB->jj;
          localB->F[P_p_tmp] += localB->P_p_m[15 * a_tmp + localB->jj] *
            localB->delta_idx_1;
        }

        localB->F_c[localB->j + 15 * a_tmp] = localB->P_p[a_tmp_0] - localB->
          G_k[a_tmp_0];
      }
    }

    for (localB->j = 0; localB->j < 6; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->A_tmp[a_tmp + 15 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 6; a_tmp++) {
        localB->delta_idx_1 = localB->R[6 * localB->j + a_tmp];
        for (a_tmp_0 = 0; a_tmp_0 <= 8; a_tmp_0 += 4) {
          tmp_0 = _mm_loadu_ps(&localB->X[15 * a_tmp + a_tmp_0]);
          localB->jj = 15 * localB->j + a_tmp_0;
          tmp = _mm_loadu_ps(&localB->A_tmp[localB->jj]);
          _mm_storeu_ps(&localB->A_tmp[localB->jj], _mm_add_ps(_mm_mul_ps(tmp_0,
            _mm_set1_ps(localB->delta_idx_1)), tmp));
        }

        for (a_tmp_0 = 12; a_tmp_0 < 15; a_tmp_0++) {
          P_p_tmp = 15 * localB->j + a_tmp_0;
          localB->A_tmp[P_p_tmp] += localB->X[15 * a_tmp + a_tmp_0] *
            localB->delta_idx_1;
        }
      }
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        localB->P_p[a_tmp + 15 * localB->j] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        a_tmp_0 = 15 * localB->j + a_tmp;
        localB->delta_idx_1 = localB->F_c[a_tmp_0];
        for (localB->jj = 0; localB->jj <= 8; localB->jj += 4) {
          tmp_0 = _mm_loadu_ps(&localB->F[15 * a_tmp + localB->jj]);
          P_p_tmp = 15 * localB->j + localB->jj;
          tmp = _mm_loadu_ps(&localB->P_p[P_p_tmp]);
          _mm_storeu_ps(&localB->P_p[P_p_tmp], _mm_add_ps(_mm_mul_ps(tmp_0,
            _mm_set1_ps(localB->delta_idx_1)), tmp));
        }

        for (localB->jj = 12; localB->jj < 15; localB->jj++) {
          P_p_tmp = 15 * localB->j + localB->jj;
          localB->P_p[P_p_tmp] += localB->F[15 * a_tmp + localB->jj] *
            localB->delta_idx_1;
        }

        localB->G_k[a_tmp_0] = 0.0F;
      }

      for (a_tmp = 0; a_tmp < 6; a_tmp++) {
        localB->delta_idx_1 = localB->X[15 * a_tmp + localB->j];
        for (a_tmp_0 = 0; a_tmp_0 <= 8; a_tmp_0 += 4) {
          tmp_0 = _mm_loadu_ps(&localB->A_tmp[15 * a_tmp + a_tmp_0]);
          localB->jj = 15 * localB->j + a_tmp_0;
          tmp = _mm_loadu_ps(&localB->G_k[localB->jj]);
          _mm_storeu_ps(&localB->G_k[localB->jj], _mm_add_ps(_mm_mul_ps(tmp_0,
            _mm_set1_ps(localB->delta_idx_1)), tmp));
        }

        for (a_tmp_0 = 12; a_tmp_0 < 15; a_tmp_0++) {
          P_p_tmp = 15 * localB->j + a_tmp_0;
          localB->G_k[P_p_tmp] += localB->A_tmp[15 * a_tmp + a_tmp_0] *
            localB->delta_idx_1;
        }
      }
    }

    for (localB->j = 0; localB->j <= 220; localB->j += 4) {
      tmp_0 = _mm_loadu_ps(&localB->P_p[localB->j]);
      tmp = _mm_loadu_ps(&localB->G_k[localB->j]);
      _mm_storeu_ps(&localB->F[localB->j], _mm_add_ps(tmp_0, tmp));
    }

    for (localB->j = 224; localB->j < 225; localB->j++) {
      localB->F[localB->j] = localB->P_p[localB->j] + localB->G_k[localB->j];
    }

    for (localB->j = 0; localB->j < 15; localB->j++) {
      for (a_tmp = 0; a_tmp < 15; a_tmp++) {
        a_tmp_0 = 15 * localB->j + a_tmp;
        localB->P_c[a_tmp_0] = (localB->F[15 * a_tmp + localB->j] + localB->
          F[a_tmp_0]) / 2.0F;
      }
    }

    std::memcpy(&localDW->UnitDelay1_DSTATE[0], &localB->P_c[0], 225U * sizeof
                (real32_T));
    std::memcpy(&localDW->UnitDelay_DSTATE[0], &localB->x_c[0], sizeof(real32_T)
                << 4U);
  }

  // End of Switch: '<Root>/Switch1'
  // End of Outputs for SubSystem: '<Root>/EKFUpdate'

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Attitude_quat[0] = localDW->UnitDelay_DSTATE[6];
  rty_StateEstBus_Attitude_quat[1] = localDW->UnitDelay_DSTATE[7];
  rty_StateEstBus_Attitude_quat[2] = localDW->UnitDelay_DSTATE[8];
  rty_StateEstBus_Attitude_quat[3] = localDW->UnitDelay_DSTATE[9];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Position_NED[0] = localDW->UnitDelay_DSTATE[0];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Velocity_NED[0] = localDW->UnitDelay_DSTATE[3];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   BusCreator generated from: '<Root>/BiasCorrectedInertialOutput'
  //   MATLAB Function: '<Root>/BiasCorrectedInertialOutput'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_AngularRate_Bod[0] = rtu_IMU_BUS_Gyro[0] -
    localDW->UnitDelay_DSTATE[13];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   BusCreator generated from: '<Root>/BiasCorrectedInertialOutput'
  //   MATLAB Function: '<Root>/BiasCorrectedInertialOutput'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Accel_Body[0] = rtu_IMU_BUS_Accel[0] -
    localDW->UnitDelay_DSTATE[10];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_GyroBias[0] = localDW->UnitDelay_DSTATE[13];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_AccelBias[0] = localDW->UnitDelay_DSTATE[10];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'

  rty_StateEstBus_Wind_NED[0] = 0.0F;

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Position_NED[1] = localDW->UnitDelay_DSTATE[1];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Velocity_NED[1] = localDW->UnitDelay_DSTATE[4];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   BusCreator generated from: '<Root>/BiasCorrectedInertialOutput'
  //   MATLAB Function: '<Root>/BiasCorrectedInertialOutput'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_AngularRate_Bod[1] = rtu_IMU_BUS_Gyro[1] -
    localDW->UnitDelay_DSTATE[14];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   BusCreator generated from: '<Root>/BiasCorrectedInertialOutput'
  //   MATLAB Function: '<Root>/BiasCorrectedInertialOutput'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Accel_Body[1] = rtu_IMU_BUS_Accel[1] -
    localDW->UnitDelay_DSTATE[11];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_GyroBias[1] = localDW->UnitDelay_DSTATE[14];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_AccelBias[1] = localDW->UnitDelay_DSTATE[11];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'

  rty_StateEstBus_Wind_NED[1] = 0.0F;

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Position_NED[2] = localDW->UnitDelay_DSTATE[2];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Velocity_NED[2] = localDW->UnitDelay_DSTATE[5];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   BusCreator generated from: '<Root>/BiasCorrectedInertialOutput'
  //   MATLAB Function: '<Root>/BiasCorrectedInertialOutput'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_AngularRate_Bod[2] = rtu_IMU_BUS_Gyro[2] -
    localDW->UnitDelay_DSTATE[15];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   BusCreator generated from: '<Root>/BiasCorrectedInertialOutput'
  //   MATLAB Function: '<Root>/BiasCorrectedInertialOutput'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_Accel_Body[2] = rtu_IMU_BUS_Accel[2] -
    localDW->UnitDelay_DSTATE[12];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_GyroBias[2] = localDW->UnitDelay_DSTATE[15];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'
  //   UnitDelay: '<Root>/Unit Delay'

  rty_StateEstBus_AccelBias[2] = localDW->UnitDelay_DSTATE[12];

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   MATLAB Function: '<Root>/StatePack'

  rty_StateEstBus_Wind_NED[2] = 0.0F;

  // SignalConversion generated from: '<Root>/StateEstBus' incorporates:
  //   Constant: '<Root>/EstimatorStatus'
  //   MATLAB Function: '<Root>/StatePack'

  *rty_StateEstBus_Status = 1U;
}

// Model initialize function
void EKF_initialize(const char_T **rt_errorStatus, RT_MODEL_EKF_T *const EKF_M)
{
  // Registration code

  // initialize error status
  EKF_M->setErrorStatusPointer(rt_errorStatus);
}

const char_T* RT_MODEL_EKF_T::getErrorStatus() const
{
  return (*(errorStatus));
}

void RT_MODEL_EKF_T::setErrorStatus(const char_T* const aErrorStatus) const
{
  (*(errorStatus) = aErrorStatus);
}

const char_T** RT_MODEL_EKF_T::getErrorStatusPointer() const
{
  return errorStatus;
}

void RT_MODEL_EKF_T::setErrorStatusPointer(const char_T** aErrorStatusPointer)
{
  (errorStatus = aErrorStatusPointer);
}

//
// File trailer for generated code.
//
// [EOF]
//
