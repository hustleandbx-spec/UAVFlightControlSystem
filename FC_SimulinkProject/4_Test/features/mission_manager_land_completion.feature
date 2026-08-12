# --- front-matter:toml ---
# 本文件验证 MissionManager 降落完成上报（MM-014/015/016）：末段降落完成后，任务以
# LandingCompleted=1、ExecutionPhase=3、MissionOutcome=1 结束，触发 Commander 加锁与 FC 电机停转。
# 触地判据（垂向速度/旋翼卸载/无水平运动/无旋转 + 高度门限 + 指令下降无实际垂直运动）为 MM-015
# 多判据集，其模型驱动信号（状态/动力卸载）为 M2 L3 接口项（本文件锁定可观测输出契约）。
# 本文件以"降落段设计终止态到达"（NavigationStatus 报悬停保持）代理触地完成信号，M2 对齐判据后转绿。
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
ExecutionPhase = "MissionStatus.ExecutionPhase"
MissionOutcome = "MissionStatus.MissionOutcome"
MissionReason = "MissionStatus.MissionReason"
LandingCompleted = "MissionStatus.LandingCompleted"
ContractValid = "NavigationContract.Valid"
StatusValid = "MissionStatus.Valid"
# --- end front-matter ---

Feature: MissionManager final landing completion
  A landing completion becomes a mission fact only when the landing segment's design terminal state is
  reached (MM-014/015/016), ending the mission with a successful outcome and the landing-completed flag.

Scenario: Landing completion ends the mission successfully
  Given inputs
    * PlanId = const(301)
    * WaypointCount = const(1)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(301)
    * RequestCommandId = const(301)
    * RequestAction = step(0 -> 1 @ 0.5s)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * Armed = const(1)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = step(1 -> 2 @ 1s)
    * NavValid = const(1)
  When simulate for 2s in Normal mode
  Then outputs
    * MissionEndsAfterLand: ExecutionPhase == 3 when t > 1.2s
    * MissionSucceedsAfterLand: MissionOutcome == 1 when t > 1.2s
    * ReasonIsRouteComplete: MissionReason == 1 when t > 1.2s
    * LandingFactIsPublished: LandingCompleted == 1 when t > 1.2s

Scenario: Landing completion is withheld until the terminal state
  Given inputs
    * PlanId = const(302)
    * WaypointCount = const(1)
    * FinishAction = const(2)
    * SafeTakeoffHeight = const(3)
    * DepartureSpeed = const(2)
    * DefaultSpeed = const(3)
    * PassMode1 = const(0)
    * PassMode2 = const(0)
    * PlanValid = const(1)
    * RequestId = const(302)
    * RequestCommandId = const(302)
    * RequestAction = step(0 -> 1 @ 0.5s)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * Armed = const(1)
    * CommanderCurrentMode = const(0)
    * CommanderValid = const(1)
    * NavGuidanceState = const(1)
    * NavValid = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * MissionInProgress: ExecutionPhase == 1 when t > 0.6s
    * NotLandedYet: LandingCompleted == 0 when t > 0.6s
