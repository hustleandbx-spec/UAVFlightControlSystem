# MissionManager Test Plan

## Status: Approved for component MIL

**Last Updated:** 2026-08-01  
**Architecture:** [MissionManager architecture](mission-manager-architecture.md)

## 1. Validation stages

1. Structural and protocol-isolation contract.
2. Stateflow lint and model compilation.
3. Integrated standalone MissionManager MIL.
4. Decision coverage review.
5. Deferred Navigator/Gateway/Gazebo integration MIL.

## 2. Component checks

| Component | Check | Acceptance criterion |
|---|---|---|
| `MissionExecutive` | Default initialization | `NO_PLAN`, invalid objective, reached index `-1` |
| `MissionExecutive` | Authorized Start | `READY -> RUNNING`, `ObjectiveId` increments |
| `MissionExecutive` | Authorization loss | `RUNNING -> FAILED` within one execution tick |
| `MissionExecutive` | Correlated completion | Only matching valid `ObjectiveId` advances |
| `ObjectiveBuilder` | Indexed item selection | Target fields match the selected zero-based item |
| `ObjectiveBuilder` | Terminal Hold | Type is `3`, final target is preserved |
| `StatusBuilder` | State mapping | Completed and Failure flags are mutually exclusive and follow State |

## 3. Integrated MIL scenarios

The executable source is
`FC_SimulinkProject/4_Test/mission_manager.feature`.

| Scenario | Acceptance criterion |
|---|---|
| Authorized two-item mission advances to second item | State running, index 1, first item reached, second target emitted |
| Authorized one-item mission completes and holds | State completed, Hold objective valid |
| Start rejected while disarmed | State remains ready, objective invalid |
| Unsupported item fails closed | State failed, failure true, objective invalid |
| Start with wrong PlanId rejected | State remains ready, objective invalid |

## 4. Fault and sequence scenarios

The executable regression now covers:

- Active PlanId changes while running.
- Commander mission authorization is lost while running.
- Mismatched NavigationStatus is ignored.
- Duplicate RequestId is ignored.
- A new Cancel from Running returns Ready.
- AutoContinue false holds the current running item after reached feedback.
- Gateway mission identity, item metadata, yaw, acceptance radius, and hold
  time are projected into the structured navigation objective.

Still deferred: explicit plan-invalid transition coverage, every Commander
invalid/disarmed/failsafe combination, Cancel from Completed, and recovery from
`FAILED`.

## 5. Simulation configuration

| Setting | Value |
|---|---|
| Solver | Fixed-step discrete |
| Sample time | `CMD_SAMPLE_TIME` |
| Simulation mode | Normal |
| Input dimensions and types | Compiled Bus definitions from `GlobalTypes.sldd` |
| Coverage | Decision |

## 6. Pass criteria

- Update Diagram succeeds.
- Static contract succeeds with no protocol leakage.
- All existing MIL scenarios and assessments pass.
- No regression from the executable prototype behavior.
- Coverage shortfalls are recorded and mapped to the deferred fault scenarios;
  they do not justify adding behavior that is outside V0.

## 7. Latest verification

| Suite | Result |
|---|---:|
| Main MissionManager Normal-mode MIL | 11/11 scenarios, 45/45 assessments |
| Structured NavigationObjective MIL | 1/1 scenario, 9/9 assessments |
| Decision coverage | 65% (35/54) |
| Static boundary/protocol contract | PASS |
| Gateway mission-plan adapter | PASS |
| Commander command audit and QGC separation | PASS |
| MAVLink UDP loopback | PASS |
