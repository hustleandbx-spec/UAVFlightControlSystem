# UAV_FC_loop Closed-Loop Compile Recovery Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore successful update compilation of `UAV_FC_loop.slx` while preserving its fixed-step EKF integration baseline.

**Architecture:** Treat the existing integration tests as executable interface contracts. Establish the current first failure, isolate it to one model boundary, add a focused regression assertion, apply the smallest source-controlled model/script change, and rerun the complete integration gate. Unknown downstream failures receive their own targeted plan after they are observed rather than speculative edits.

**Tech Stack:** MATLAB R2025b, Simulink Model Reference, Simulink data dictionaries, MATLAB function-based integration tests.

---

### Task 1: Capture the Current Closed-Loop Compile Baseline

**Files:**
- Test: `FC_SimulinkProject/4_Test/test_ekf_integration.m`
- Test: `FC_SimulinkProject/4_Test/test_flight_control_integration.m`
- Test: `FC_SimulinkProject/4_Test/test_sensor_sample_time_contract.m`
- Inspect: `FC_SimulinkProject/2_Model/control/UAV_FlightControl.slx`
- Inspect: `FC_SimulinkProject/2_Model/state_estimation/EKF/EKF.slx`
- Inspect: `FC_SimulinkProject/3_Simulation/UAV_FC_loop.slx`

- [ ] **Step 1: Verify saved solver contracts offline**

Read each model's `simulink/configSet*.xml` and require:

```text
UAV_FlightControl: SolverName=FixedStepDiscrete, FixedStep=auto
EKF:               SolverName=FixedStepDiscrete, FixedStep=auto
UAV_FC_loop:       SolverName=FixedStepDiscrete, FixedStep=0.001
```

- [ ] **Step 2: Run the focused MATLAB integration tests**

Run:

```powershell
& 'D:\MATLAB\R2025b\bin\matlab.exe' -batch "openProject('D:\Project\UAVSingleFlightControl\FC_SimulinkProject\FC_SimulinkProject.prj'); test_flight_control_integration; test_sensor_sample_time_contract; test_ekf_integration; disp('CLOSED_LOOP_COMPILE_GATE_PASS');"
```

Expected on success: MATLAB exits with status 0 and prints `CLOSED_LOOP_COMPILE_GATE_PASS`.

Expected on failure: MATLAB exits nonzero or reports the first Simulink diagnostic before the pass marker.

- [ ] **Step 3: Classify the first observed result**

If all tests pass, proceed directly to Task 3. If a test fails, record exactly one first root-cause category and proceed to Task 2:

```text
solver/model-reference compatibility
sample-time propagation
Bus data type or dimensions
data-dictionary parameter resolution
model loading or project-path setup
other explicit Simulink diagnostic
```

### Task 2: Repair One Observed Compile Contract Failure

**Files:**
- Modify: the single model source, YAML parameter source, model construction script, or integration test directly implicated by Task 1
- Test: the existing `FC_SimulinkProject/4_Test/test_*.m` file nearest to that contract

- [ ] **Step 1: Reproduce only the failing boundary**

Run the narrowest existing test that reaches the failure. Expected: the same diagnostic appears without unrelated downstream errors.

- [ ] **Step 2: Add a regression assertion before the failing update compile**

Add an assertion that describes the observed contract using `get_param`, data-dictionary entry inspection, or explicit Bus metadata inspection. Expected before repair: the assertion or subsequent update compile fails for the same root cause.

- [ ] **Step 3: Apply the minimum repair at the authoritative source**

Use these routing rules:

```text
parameter value/type  -> edit ParamSources YAML, then run create_*Dict
Bus definition        -> edit 1_Data_Dictionaries/BusConfig/config_*Bus.m, then run create_GlobalTypes
model configuration   -> edit the owning .slx model configuration
generated model block -> edit its build/refactor script and regenerate the model
hand-authored block   -> edit only that model block
```

Do not directly edit `.sldd` entries, ESKF/UKF, or AirSim assets.

- [ ] **Step 4: Rerun the narrow test**

Expected: the focused regression assertion and model update compile pass.

- [ ] **Step 5: Return to Task 1 Step 2**

If a different downstream compile failure appears, stop after recording it and add a new targeted repair task to this plan before changing another subsystem.

### Task 2A: Isolate the Model-Reference Algebraic Loop

**Files:**
- Inspect: `FC_SimulinkProject/3_Simulation/UAV_FC_loop.slx`
- Inspect: the root input/output paths of the five referenced subsystem models
- Test: `FC_SimulinkProject/4_Test/test_ekf_integration.m`

- [ ] **Step 1: Capture the top-level feedback path**

Confirm the saved model contains the exact cycle:

```text
Dynamics -> Sensor -> Estimator -> Controller -> PowerSystem -> Dynamics
```

- [ ] **Step 2: Ask Simulink for the compiled algebraic-loop membership**

Load `UAV_FC_loop` and call `Simulink.BlockDiagram.getAlgebraicLoops` with algebraic-loop diagnostics enabled. Record the blocks participating in the first loop and the first referenced model whose direct-feedthrough behavior prevents compilation.

- [ ] **Step 3: Trace state ownership at each boundary**

Inspect the five referenced models for the first stateful block between root input and root output. Determine whether the physical state belongs to Dynamics, PowerSystem, Sensor timing, or Estimator. Do not insert a delay until this ownership is established.

- [ ] **Step 4: Add a focused failing regression assertion**

Add the narrowest test that expresses the intended state boundary. Run it before changing the model and verify it fails because the feedback path remains direct-feedthrough.

- [ ] **Step 5: Implement one state-boundary repair and rerun the gate**

Modify only the model that owns the missing physical/discrete state. Rerun the focused assertion, then the complete Task 1 Step 2 command. If another downstream diagnostic appears, record it before any further modification.

### Task 3: Verify the Closed-Loop Compile Gate

**Files:**
- Test: `FC_SimulinkProject/4_Test/test_flight_control_integration.m`
- Test: `FC_SimulinkProject/4_Test/test_sensor_sample_time_contract.m`
- Test: `FC_SimulinkProject/4_Test/test_ekf_integration.m`

- [ ] **Step 1: Rerun all three integration tests in a clean MATLAB process**

Run the Task 1 Step 2 command again.

Expected: exit status 0 and `CLOSED_LOOP_COMPILE_GATE_PASS`.

- [ ] **Step 2: Recheck solver contracts from the saved `.slx` files**

Expected: all three values still match Task 1 Step 1; no fix may silently revert the solver configuration.

- [ ] **Step 3: Record project state**

Update:

```text
D:\PBOS\runtime\handoffs\UAVSingle.md
D:\PBOS\runtime\projects\UAVSingle.md
D:\PBOS\runtime\agent_handoff.md
D:\Project\UAVSingleFlightControl\开发进度.md
```

Record the exact test command and whether the next blocker is short-duration hover simulation or a newly observed compile diagnostic.

This workspace is not a Git repository, so commit steps are intentionally omitted; no claim of version-controlled rollback is made.
