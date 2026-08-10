# MissionManager System Specification

## Status: Approved for V0 implementation

**Last Updated:** 2026-08-01  
**Owner:** FlightCore command architecture

## 1. Purpose

MissionManager is the sole owner of mission execution state and mission-item
progress. It consumes a protocol-neutral mission snapshot, execution requests,
Commander authorization, and Navigator completion feedback. It emits one
task-independent navigation objective and authoritative mission status.

## 2. V0 goals

| ID | Goal | Acceptance criterion |
|---|---|---|
| MM-G1 | Execute a fixed mission of at most 10 items | A valid one- or two-item MIL mission reaches `COMPLETED` |
| MM-G2 | Enforce command authority | Start is rejected unless Commander is valid, armed, authorizes mission execution, and is not in failsafe |
| MM-G3 | Prevent stale feedback from advancing the mission | Only a matching valid `ObjectiveId` can advance `CurrentItemIndex` |
| MM-G4 | Fail closed | Plan replacement, authorization loss, or unsupported item type during execution enters `FAILED` |
| MM-G5 | Preserve protocol isolation | No MAVLink, QGC, ROS 2, Gazebo, or transport semantics occur inside the model |

## 3. Non-goals

- Mission persistence, dual banks, partial uploads, and item-by-item repository access.
- Global-coordinate conversion; Gateway must supply local NED targets.
- Trajectory generation and target-reached calculation; these belong to Navigator.
- Pause, Resume, SetCurrent, Land, RTL, `DO_JUMP`, camera commands, and continuous fly-through.
- Gateway, Commander, Navigator, FlightCore, or Gazebo integration in this increment.

## 4. External interface

All ports execute at `CMD_SAMPLE_TIME` using fixed-step discrete scheduling.
Bus definitions are authoritative in
`FC_SimulinkProject/1_Data_Dictionaries/BusConfig/`.

| Port | Direction | Type | Contract |
|---|---|---|---|
| `MissionPlan` | Input | `Bus: MissionPlanBus` | Complete, protocol-neutral snapshot; `ItemCount` is 1..10 |
| `MissionControlRequest` | Input | `Bus: MissionControlRequestBus` | Event request; duplicate `RequestId` is ignored |
| `CommanderStatus` | Input | `Bus: CommanderStatusBus` | Safety and execution authorization |
| `NavigationStatus` | Input | `Bus: NavigationStatusBus` | Task-neutral physical navigation facts correlated by `ObjectiveId` |
| `NavigationObjective` | Output | `Bus: NavigationObjectiveBus` | Task-neutral physical navigation objective compiled from the active mission phase |
| `MissionStatus` | Output | `Bus: MissionStatusBus` | Authoritative plan, state, progress, completion, and failure |

NED position is metres, yaw is radians, acceptance radius is metres, and hold
time is seconds. Mission item and status indexes are zero-based.

## 5. Operating modes

| State | Meaning | Objective behavior |
|---|---|---|
| `NO_PLAN` (0) | No accepted mission | Invalid |
| `READY` (1) | Valid mission loaded, not executing | Invalid |
| `RUNNING` (2) | Current item is executing | Valid physical navigation objective |
| `COMPLETED` (3) | Final item reached | Continue issuing the final zero-terminal-speed position objective |
| `FAILED` (4) | Execution cannot safely continue; latched until a later recovery design is approved | Invalid |

`NO_PLAN` is the default state. Mission status remains valid in every state.

## 6. Observable scenarios

| ID | Scenario | Expected result |
|---|---|---|
| MM-S1 | Valid plan and authorized Start | `READY -> RUNNING`, first `ObjectiveId` issued |
| MM-S2 | Matching reached feedback with another auto-continue item | Index and `ObjectiveId` increment |
| MM-S3 | Matching reached feedback on final item | `RUNNING -> COMPLETED`, Hold emitted |
| MM-S4 | Start while disarmed or with mismatched PlanId | Remain `READY` |
| MM-S5 | Unsupported current item | Enter `FAILED`, objective invalid |
| MM-S6 | Authorization or active plan lost while running | Enter `FAILED`, objective invalid |
| MM-S7 | Duplicate request identifier | Request is consumed once and cannot trigger a later action |
| MM-S8 | Navigation feedback with a different `ObjectiveId` | Remain on the current item |
| MM-S9 | New Cancel while running | Return to `READY`, invalidate the objective, preserve the loaded plan |
| MM-S10 | Current item reached with `AutoContinue=false` | Remain `RUNNING` on the same item and objective |

## 7. Execution constraints

- `Takeoff`, `Waypoint`, `Land`, `RTL`, and `Hold` are MissionManager task phases and must never appear in the Navigator interface.
- Single rate, fixed-step discrete, 1 ms current project base rate.
- Fixed-size arrays only; no dynamic allocation.
- State is owned only by the Stateflow executive.
- Plan receipt, command admission, objective issue, and navigation completion are
  separate events. Initialization may load and start a valid plan, but it must
  not consume navigation completion before an objective has been issued.
- Tunable calibration parameters are not required for V0.
- Generated-code readiness requires explicit integer and Boolean types and no
  protocol or runtime API dependencies.

## 8. Deferred decisions

Pause/Resume semantics, recovery from `FAILED`, plan replacement after
`COMPLETED`, and Land/RTL item semantics require a later interface and behavior
review.
