# MissionManager Architecture Specification

## Status: Approved for V0 implementation

**Last Updated:** 2026-08-01  
**Parent:** [MissionManager system specification](mission-manager-system.md)

## 1. Functional decomposition

```text
MissionPlan ───────────────┬──────────────> ObjectiveBuilder ──> NavigationObjective
MissionControlRequest ─┐   │                         ▲
CommanderStatus ───────┼──> MissionExecutive ───────┤
NavigationStatus ──────┘        (Stateflow)          └── execution context
                                  │
                                  └──────────────> StatusBuilder ─────> MissionStatus
```

The architecture separates stateful mission supervision from stateless data
selection and output assembly. No component other than `MissionExecutive`
stores plan progress or request history.

## 2. Component catalog

| Component | Implementation | Responsibility | State | DFT |
|---|---|---|---|---|
| `MissionExecutive` | Stateflow Chart | Plan acceptance, request deduplication, authorization, state transitions, item progression, correlation IDs | Yes | Partial |
| `ObjectiveBuilder` | Simulink Subsystem with a small stateless MATLAB Function leaf | Select the indexed fixed-array item and assemble `NavigationObjectiveBus` | No | Yes |
| `StatusBuilder` | Simulink Subsystem using typed conversion and Bus Creator blocks | Assemble `MissionStatusBus` from executive context | No | Yes |

The MATLAB Function leaf is permitted only for fixed-array row selection and
bus packing. It has no persistent data, no state transition logic, no command
authorization logic, and no mission progression logic.

## 3. MissionExecutive outputs

| Signal | Type | Meaning |
|---|---|---|
| `State` | `uint8` | Values 0..4 from the system specification |
| `ActivePlanId` | `uint32` | Accepted plan identifier |
| `ItemCount` | `uint8` | Accepted mission length |
| `CurrentItemIndex` | `uint8` | Zero-based active item |
| `ReachedItemIndex` | `int16` | Latest reached item, `-1` initially |
| `ObjectiveId` | `uint32` | Correlation identifier for Navigator |
| `ObjectiveValid` | `boolean` | True only in `RUNNING` or `COMPLETED` |

## 4. State ownership and transitions

### State-local behavior

- `NO_PLAN`: clears active plan metadata and progress.
- `READY`: holds accepted plan metadata and waits for a new Start request.
- `RUNNING`: enforces authorization and plan identity, validates the current
  item type, and consumes matching completion feedback.
- `COMPLETED`: owns the completed task phase, continues issuing the final physical position objective, and accepts Cancel.
- `FAILED`: invalidates the objective and retains diagnostic execution context.

### Transition priority

Within a model tick, safety conditions have priority over mission progress:

1. running plan identity/validity failure;
2. Commander authorization/failsafe failure;
3. unsupported current item;
4. new Cancel request;
5. matching target-reached feedback;
6. new Start request while ready;
7. new valid plan loading while not running.

The chart uses Moore-style state outputs for objective validity. Feedback from
Navigator is correlated through an explicit `ObjectiveId`; there is no
algebraic feedback path in the standalone component.

The default transition enters an initialization decision junction. The first
model invocation may accept an already-present validated plan and authorized
Start request, but initialization never consumes `NavigationStatus`. Mission
progress is evaluated only after the chart has entered `RUNNING` and issued the
correlated objective.

## 5. ObjectiveBuilder behavior

- Convert zero-based `CurrentItemIndex` to a bounded one-based array index.
- Select one row from each fixed-size plan array.
- When authorized, compile the selected task item into task-neutral target geometry and physical completion constraints.
- In `COMPLETED`, continue issuing the final physical position objective without exposing a Hold task type.
- When `ObjectiveValid=false`, output deterministic zero target fields and
  `Valid=false`.
- The component is stateless and does not decide whether a mission may run.
- `PlanId`, `ItemIndex`, position, yaw, acceptance radius, and `RequiredStableTime` are
  projected without MAVLink or Gateway metadata leaking into the output.
- `Takeoff`, `Waypoint`, `Land`, `RTL`, and `Hold` must not occur in `NavigationObjectiveBus`.

## 6. StatusBuilder behavior

The subsystem maps executive context directly to `MissionStatusBus`.
`Completed` is true only when `State==3`; `Failure` is true only when
`State==4`; `Valid` is always true after model initialization.

## 7. Data and numerical constraints

- All mission geometry uses `single`; identifiers and indexes use explicit
  integer types.
- Array capacity is fixed at 10 and indexing is bounded to 0..9.
- No integrators, saturation loops, continuous states, or multi-rate paths
  exist in V0.
- Parameters and Bus objects remain in project data dictionaries; no numeric
  mission constants are stored as tunable block parameters.

## 8. Key decisions

| Decision | Choice | Rationale |
|---|---|---|
| Supervisory logic representation | Stateflow | Makes modes, ownership, transition guards, and fault priority reviewable |
| Objective selection | Stateless leaf component | Isolates unavoidable fixed-array indexing from mission state |
| Status generation | Graphical Simulink subsystem | Direct mapping is visible and requires no procedural code |
| External interfaces | Unchanged | Internal MBD refactor must not ripple into neighboring components |

## 9. API verification notes

- Stateflow object creation, transition wiring, MATLAB action language, linting,
  and autolayout follow the installed Simulink Agentic Toolkit Stateflow
  reference and are verified in MATLAB R2025b.
- Simulink subsystem, Bus Creator, comparison, constant, and type-conversion
  blocks are established project/toolbox primitives.
- The stateless MATLAB Function leaf will be compile-checked by Update Diagram
  and exercised by the existing full Normal-mode MIL harness.
