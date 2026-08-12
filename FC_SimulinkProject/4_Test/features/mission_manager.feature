# --- front-matter:toml ---
# 本文件验证 MissionManager 任务状态推进（MM-002..013）：单计划槽、结构校验、批准
# "开始任务"后下发首个起飞运动学合同、进度推进、任务结束上报。
# 字段对齐 A2 契约 #5/#6 + MissionStatusBus（DEC-087/067）。
# 注意：MM 的 NED 坐标来自 A1 Lat/Lon(deg)/Height(相对参考点) 经 MM 内部转换（M2 定算法）；
# 本文件锁"合同字段面 + 状态枚举"，不锁 waypoint 几何投影（见 structured_objective.feature 说明）。
# 进度/触地推进需状态类驱动信号（M2 定接口），本文件以合同下发与阶段枚举为断言。
model = "MissionManager.slx"
[inputs]
PlanId = "MissionPlan.TaskId"
WaypointCount = "MissionPlan.WaypointCount"
FinishAction = "MissionPlan.FinishAction"
SafeTakeoffHeight = "MissionPlan.SafeTakeoffHeight"
DepartureSpeed = "MissionPlan.DepartureSpeed"
DefaultSpeed = "MissionPlan.DefaultSpeed"
PassMode1 = "MissionPlan.PassMode(1)"
PassMode2 = "MissionPlan.PassMode(2)"
Speed1 = "MissionPlan.Speed(1)"
Speed2 = "MissionPlan.Speed(2)"
PlanValid = "MissionPlan.Valid"
RequestId = "MissionControlRequest.RequestId"
RequestCommandId = "MissionControlRequest.CommandId"
RequestAction = "MissionControlRequest.Action"
RequestValid = "MissionControlRequest.Valid"
Armed = "CommanderStatus.Armed"
CommanderCurrentMode = "CommanderStatus.CurrentMode"
CommanderValid = "CommanderStatus.Valid"
NavGuidanceState = "NavigationStatus.GuidanceState"
NavValid = "NavigationStatus.Valid"
[outputs]
ContractId = "NavigationContract.ContractId"
ContractPlanId = "NavigationContract.PlanId"
ContractSegmentIndex = "NavigationContract.SegmentIndex"
ContractStartValid = "NavigationContract.StartValid"
ContractPassCount = "NavigationContract.PassCount"
ContractEndDown = "NavigationContract.EndPositionNED(3)"
ContractCruiseSpeed = "NavigationContract.CruiseSpeed"
ContractValid = "NavigationContract.Valid"
StatusPlanId = "MissionStatus.PlanId"
StatusPlanValid = "MissionStatus.PlanValid"
ExecutionPhase = "MissionStatus.ExecutionPhase"
MissionOutcome = "MissionStatus.MissionOutcome"
MissionReason = "MissionStatus.MissionReason"
ActiveContractId = "MissionStatus.ActiveContractId"
LandingCompleted = "MissionStatus.LandingCompleted"
StatusValid = "MissionStatus.Valid"
# --- end front-matter ---

Feature: MissionManager mission state progression on the v2 contract surface
  MissionManager holds the single mission plan slot (MM-002), validates structure (MM-003), and on an
  approved start issues the takeoff kinematic contract (MM-007). It advances segments by a pre-partitioned
  lookup and reports state via MissionStatusBus (MM-006/009/018).

Scenario: Valid two-stop plan is accepted and idles ready
  Given inputs
    * PlanId = const(11)
    * WaypointCount = const(2)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * Speed1 = const(0)
    * Speed2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(0)
    * RequestCommandId = const(0)
    * RequestAction = const(0)
    * RequestValid = const(0)
    * Armed = const(0)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(0)
    * NavValid = const(1)
  When simulate for 10ms in Normal mode
  Then outputs
    * PlanAccepted: StatusPlanValid == 1
    * PlanCorrelatesTask: StatusPlanId == 11
    * IdlePhase: ExecutionPhase == 0
    * NoContractUntilStart: ContractValid == 0

Scenario: Approved start issues the takeoff contract
  Given inputs
    * PlanId = const(12)
    * WaypointCount = const(2)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * Speed1 = const(0)
    * Speed2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(1)
    * RequestCommandId = const(10)
    * RequestAction = step(0 -> 1 @ 0.5s)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * Armed = const(1)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(0)
    * NavValid = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * ExecutionStarted: ExecutionPhase == 1 when t > 0.6s
    * ContractIssued: ContractValid == 1 when t > 0.6s
    * ContractCorrelatesPlan: ContractPlanId == 12 when t > 0.6s
    * ContractTracked: ActiveContractId > 0 when t > 0.6s
    * TakeoffClimbsToSafeHeight: ContractEndDown == -3 when t > 0.6s
    * TakeoffSegmentSimple: ContractPassCount == 0 when t > 0.6s
    * ContractAnchoredToStart: ContractStartValid == 1 when t > 0.6s
    * StatusValid: StatusValid == 1 when t > 0.6s

Scenario: Start while disarmed stays ready
  Given inputs
    * PlanId = const(13)
    * WaypointCount = const(1)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * Speed1 = const(0)
    * Speed2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(2)
    * RequestCommandId = const(11)
    * RequestAction = step(0 -> 1 @ 0.5s)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * Armed = const(0)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(0)
    * NavValid = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * RemainsReady: ExecutionPhase == 0 when t > 0.6s
    * NoContractWhileDisarmed: ContractValid == 0 when t > 0.6s

Scenario: Plan whose last waypoint is not a stop is rejected
  Given inputs
    * PlanId = const(14)
    * WaypointCount = const(1)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(1)
    * PassMode2 = const(0)
    * Speed1 = const(0)
    * Speed2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(0)
    * RequestCommandId = const(0)
    * RequestAction = const(0)
    * RequestValid = const(0)
    * Armed = const(0)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(0)
    * NavValid = const(1)
  When simulate for 10ms in Normal mode
  Then outputs
    * PlanRejected: StatusPlanValid == 0
    * NoContractOnReject: ContractValid == 0
