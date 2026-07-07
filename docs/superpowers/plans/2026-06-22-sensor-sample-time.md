# Sensor Sample-Time Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make sensor scheduling parameters satisfy Simulink's `double` sample-time contract while preserving single-precision sensor data and noise parameters.

**Architecture:** Treat the Simulink scheduler as a type boundary. Cast frequency-derived periods to `double` at scheduling block parameters only; keep the YAML sensor-domain parameters and signal chain in `single`.

**Tech Stack:** MATLAB R2025b, Simulink, MATLAB scripts, SLX models

---

### Task 1: Add the regression test

**Files:**
- Create: `FC_SimulinkProject/4_Test/test_sensor_sample_time_contract.m`

- [ ] Add a test that update-compiles `sensor_IMU_model`.
- [ ] Run it before production changes and confirm the current White Noise sample-time error.

### Task 2: Apply the scheduling boundary

**Files:**
- Modify: `FC_SimulinkProject/2_Model/sensor_model/build_IMU_sensor.m`
- Modify: `FC_SimulinkProject/2_Model/sensor_model/build_GPS_sensor.m`
- Modify: `FC_SimulinkProject/2_Model/sensor_model/sensor_IMU_model.slx`
- Modify: `FC_SimulinkProject/2_Model/sensor_model/sensor_GPS_model.slx`

- [ ] Change only scheduling expressions from `1 / *_SAMPLE_RATE` to `1 / double(*_SAMPLE_RATE)`.
- [ ] Preserve covariance expressions and all `single` YAML parameter types.

### Task 3: Verify integration

**Files:**
- Test: `FC_SimulinkProject/4_Test/test_sensor_sample_time_contract.m`
- Test: `FC_SimulinkProject/4_Test/test_ekf_integration.m`

- [ ] Run the new regression test and confirm it passes.
- [ ] Update-compile the GPS model.
- [ ] Update-compile `UAV_FC_loop` and report the next blocker if compilation proceeds beyond sensor sample times.
