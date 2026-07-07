# Sensor Sample-Time Type Boundary Design

## Goal

Remove the sensor-model compilation blocker without weakening the single-precision sensor-data contract.

## Type boundary

- Sensor signals, noise density, bias random walk, scale error, and saturation parameters remain `single`.
- Simulink scheduling periods are compile-time, non-tunable `double` values.
- White-noise outputs are converted to `single` before entering the sensor signal chain.

## Implementation

Use `1 / double(IMU_SAMPLE_RATE)` and `1 / double(GPS_SAMPLE_RATE)` only at block parameters that define sample time. Casting must occur before division so single-precision rounding cannot produce periods that are incompatible with the fixed solver step. Keep covariance expressions and exported sensor parameters unchanged. Apply the expressions both to the programmatic model builders and to the checked-in `.slx` models.

## Verification

First reproduce the existing IMU update failure with a regression test. After the change, update-compile the IMU model, GPS model, and `UAV_FC_loop` to expose any subsequent integration blocker.
