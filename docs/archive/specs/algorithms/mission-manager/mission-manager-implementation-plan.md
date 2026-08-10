# MissionManager Implementation Plan

## Status: Complete

**Last Updated:** 2026-08-01  
**Architecture:** [MissionManager architecture](mission-manager-architecture.md)  
**Test Plan:** [MissionManager test plan](mission-manager-test-plan.md)

## 1. Model hierarchy

```text
MissionManager.slx
├─ MissionExecutive        (Stateflow)
├─ ObjectiveBuilder        (Subsystem)
│  └─ SelectMissionItem    (stateless MATLAB Function leaf)
└─ StatusBuilder           (Subsystem, graphical)
```

## 2. Build phases

| Phase | Work | Exit criterion |
|---|---|---|
| 0 | Freeze existing four inputs, two outputs, Bus definitions, state values, and sample time | Existing contract test remains unchanged and passes |
| 1 | Replace monolithic `MissionLogic` with component stubs and wire root data flow | `model_read` shows the required hierarchy |
| 2 | Implement Stateflow executive, stateless objective selection, and graphical status assembly | Update Diagram passes; Stateflow lint is clean |
| 3 | Run integrated Normal-mode MIL and coverage | All existing scenarios and assessments pass |
| 4 | Later system integration | Navigator and Gateway review gates approved |
| 5 | Harden Gateway/Commander event sequencing | No initialization-time navigation completion; expanded MIL passes |

## 3. Verification checkpoints

- After structural edit: inspect root and every component with `model_read`.
- After Stateflow edit: run `utils.sfCheckChart`, then Stateflow autolayout.
- After integration: Update Diagram with the project open and dictionary loaded.
- Regression: run all scenarios in `mission_manager.feature` in full Normal mode.
- Contract: run `test_mission_manager_contract.m` and ensure protocol terms are
  still absent.

## 4. Definition of done for this increment

- External Bus interfaces and numeric state semantics are unchanged.
- Root model visibly contains three named functional components.
- Only Stateflow owns persistent mission state.
- The array-selection MATLAB Function is stateless and contains no mission
  transition or authorization decisions.
- Status output is assembled graphically.
- Existing five MIL scenarios pass without relaxing assertions.
- Gateway mission geometry and constraints are preserved in the emitted
  `NavigationObjectiveBus`.
- Authorization loss, plan replacement, duplicate requests, correlated
  navigation feedback, Cancel, and `AutoContinue=false` have persistent MIL
  coverage.

## 5. Risks

| Risk | Mitigation |
|---|---|
| Stateflow transition priority differs from prototype | Specify safety-first priority and retain behavioral regression tests |
| Bus typing is lost during decomposition | Use dictionary Bus types at external ports and compile full Normal mode |
| Test harness metadata becomes stale after model structure changes | Rebuild through `model_test` using the existing feature file |
| Objective changes one tick later due to Moore semantics | Treat the one-tick discrete update as deterministic and verify expected outputs in MIL |

## 6. 2026-08-01 verification result

- Root interface remains four inputs and two outputs with unchanged Bus types.
- Stateflow lint and Update Diagram pass.
- Main Normal-mode MIL: 11/11 scenarios and 45/45 assessments pass.
- Structured-objective MIL: 1/1 scenario and 9/9 assessments pass.
- Decision coverage increased from 53% (34/64) to 65% (35/54) after removing
  unreachable initialization shortcuts and adding sequence/fault coverage.
- Gateway mission adapter, Commander audit, QGC separation contract, and MAVLink
  UDP loopback regressions pass.
