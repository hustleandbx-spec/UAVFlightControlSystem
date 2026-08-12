# --- front-matter:toml ---
# 本文件验证 MissionManager → NavigationContractBus v2 的段合同投影（MM-006/007/009）。
# "开始任务"后 MM 下发的首个合同是起飞段：终止停点=(home_xy, 安全起飞高度)，段内无过点。
# 投影字段锁定：PlanId 关联 / StartValid / PassCount=0 / EndPositionNED(3)←SafeTakeoffHeight / Valid。
# 说明：waypoint 几何投影（A1 Lat/Lon(deg)→NED）依赖 MM 坐标转换算法（M2 定），本文件不锁；
#       CruiseSpeed/MaxAcceleration/MaxJerk/TargetYaw 的段级映射在 M2 可驱动段推进后校验。
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
ContractEndNorth = "NavigationContract.EndPositionNED(1)"
ContractEndEast = "NavigationContract.EndPositionNED(2)"
ContractEndDown = "NavigationContract.EndPositionNED(3)"
ContractValid = "NavigationContract.Valid"
ActiveContractId = "MissionStatus.ActiveContractId"
ExecutionPhase = "MissionStatus.ExecutionPhase"
StatusValid = "MissionStatus.Valid"
# --- end front-matter ---

Feature: MissionManager projects a takeoff segment contract on the v2 surface
  The first contract issued after an approved start is the takeoff climb to the safe takeoff height
  (MM-007). The contract carries the plan correlation, anchors to the current state, and is a simple
  stop segment with no intermediate pass points.

Scenario: Approved start projects the takeoff contract fields
  Given inputs
    * PlanId = const(101)
    * WaypointCount = const(2)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(101)
    * RequestCommandId = const(101)
    * RequestAction = step(0 -> 1 @ 0.5s)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * Armed = const(1)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(0)
    * NavValid = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * ContractIssued: ContractValid == 1 when t > 0.6s
    * PlanIdentityPreserved: ContractPlanId == 101 when t > 0.6s
    * FirstSegmentIsTakeoff: ContractSegmentIndex == 0 when t > 0.6s
    * ContractAnchoredToStart: ContractStartValid == 1 when t > 0.6s
    * NoPassPointsOnTakeoff: ContractPassCount == 0 when t > 0.6s
    * TakeoffHeightProjected: ContractEndDown == -3 when t > 0.6s
    * HorizontalHomeUnspecified: ContractEndNorth == 0 when t > 0.6s
    * ContractTracked: ActiveContractId > 0 when t > 0.6s
    * StatusValid: StatusValid == 1 when t > 0.6s
