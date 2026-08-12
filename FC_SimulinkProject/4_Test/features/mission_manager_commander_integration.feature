# --- front-matter:toml ---
# 本文件验证 Commander 模式级命令经 MissionControlRequestBus 路由治理 MM 正常流（CMD-002/MM-017/020）。
# Action 集 = 1开始任务 2返航 3降落 4悬停 5接管 6go-to-point 激活（暂停/继续/取消 = 后置）。
# go-to-point 的 TargetPositionNED 直接以 NED(m) 携带目标点 → 合同 EndPositionNED 可精确断言。
# 挂起/终止等 FailSafe 语义后置，不在第一阶段正常流断言中出现。
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
PlanValid = "MissionPlan.Valid"
RequestId = "MissionControlRequest.RequestId"
RequestCommandId = "MissionControlRequest.CommandId"
RequestAction = "MissionControlRequest.Action"
RequestTargetNorth = "MissionControlRequest.TargetPositionNED(1)"
RequestTargetEast = "MissionControlRequest.TargetPositionNED(2)"
RequestTargetDown = "MissionControlRequest.TargetPositionNED(3)"
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
ContractEndNorth = "NavigationContract.EndPositionNED(1)"
ContractEndEast = "NavigationContract.EndPositionNED(2)"
ContractEndDown = "NavigationContract.EndPositionNED(3)"
ContractValid = "NavigationContract.Valid"
ExecutionPhase = "MissionStatus.ExecutionPhase"
MissionOutcome = "MissionStatus.MissionOutcome"
MissionReason = "MissionStatus.MissionReason"
ActiveContractId = "MissionStatus.ActiveContractId"
StatusValid = "MissionStatus.Valid"
# --- end front-matter ---

Feature: Commander mode-level commands govern MissionManager in the normal flow
  Approved mode-level commands reach MissionManager through MissionControlRequestBus: start begins the
  mission, return-to-launch aborts plan progression and re-routes to a home contract, and go-to-point
  interrupts the plan to navigate to a requested NED point then hover (MM-017/020).

Scenario: Start command routes the mission into execution
  Given inputs
    * PlanId = const(201)
    * WaypointCount = const(2)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(201)
    * RequestCommandId = const(201)
    * RequestAction = step(0 -> 1 @ 0.5s)
    * RequestTargetNorth = const(0)
    * RequestTargetEast = const(0)
    * RequestTargetDown = const(0)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * Armed = const(1)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(0)
    * NavValid = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * MissionActive: ExecutionPhase == 1 when t > 0.6s
    * ContractIssued: ContractValid == 1 when t > 0.6s
    * ContractCorrelatesPlan: ContractPlanId == 201 when t > 0.6s
    * NoOutcome: MissionOutcome == 0 when t > 0.6s

Scenario: Return-to-launch re-routes without terminating the mission
  Given inputs
    * PlanId = const(202)
    * WaypointCount = const(2)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(202)
    * RequestCommandId = const(202)
    * RequestAction = step(0 -> 2 @ 0.5s)
    * RequestTargetNorth = const(0)
    * RequestTargetEast = const(0)
    * RequestTargetDown = const(0)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * Armed = const(1)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(0)
    * NavValid = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * RtlReRoutesContract: ContractValid == 1 when t > 0.6s
    * MissionContinuesInRtl: ExecutionPhase == 1 when t > 0.6s
    * NoOutcomeOnRtl: MissionOutcome == 0 when t > 0.6s

Scenario: Go-to-point navigates to the requested NED point and hovers
  Given inputs
    * PlanId = const(203)
    * WaypointCount = const(2)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(203)
    * RequestCommandId = const(203)
    * RequestAction = step(0 -> 6 @ 0.5s)
    * RequestTargetNorth = const(5)
    * RequestTargetEast = const(2)
    * RequestTargetDown = const(-5)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * Armed = const(1)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(0)
    * NavValid = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * GotoPointContractIssued: ContractValid == 1 when t > 0.6s
    * ContractTargetsRequestedPoint: ContractEndNorth == 5 when t > 0.6s
    * ContractTargetEastPreserved: ContractEndEast == 2 when t > 0.6s
    * ContractTargetAltitudePreserved: ContractEndDown == -5 when t > 0.6s
    * MissionContinuesInGoto: ExecutionPhase == 1 when t > 0.6s
