# --- front-matter:toml ---
# 本文件验证 Commander 统一命令门卫（CMD-001..006）：按 CommanderRequestBus
# 命令集（CommandId/Command/Params/Valid）审查、路由 MissionControlRequestBus、
# 逐条 CommandAckBus 回报、发布 CommanderStatusBus 状态流。
# 注意：模型目标顶层接口 = CommanderRequestBus 入 + CommandAckBus 出（M2 L3 补齐；
# 当前模型入仍为扁平 CommandIngressSnapshot、无 CommandAck 出端口）。本文件锁定目标契约。
# 旧 authority/执行使能语义字段已从新 CommanderStatusBus 移除，相关旧场景一并删除。
# 命令集：1加锁 2解锁 3开始任务 4返航 5降落 6悬停 7接管 8go-to-point；9载荷控制=后置。
model = "Commander.slx"
[inputs]
RequestCommandId = "CommanderRequest.CommandId"
RequestCommand = "CommanderRequest.Command"
RequestParam1 = "CommanderRequest.Params(1)"
RequestParam2 = "CommanderRequest.Params(2)"
RequestParam3 = "CommanderRequest.Params(3)"
RequestValid = "CommanderRequest.Valid"
MissionExecutionPhase = "MissionStatus.ExecutionPhase"
MissionFaultCode = "MissionStatus.MissionFaultCode"
MissionStatusValid = "MissionStatus.Valid"
LandingCompleted = "MissionStatus.LandingCompleted"
LinkConnected = "GcsLinkStatus.Connected"
LinkValid = "GcsLinkStatus.Valid"
EstimatorStatus = "StateEstBus.Status"
[outputs]
Armed = "CommanderStatus.Armed"
CurrentMode = "CommanderStatus.CurrentMode"
LastCommandResult = "CommanderStatus.LastCommandResult"
LastCommandId = "CommanderStatus.LastCommandId"
SafetyState = "CommanderStatus.SafetyState"
SafetyDirective = "CommanderStatus.SafetyDirective"
MissionDirective = "CommanderStatus.MissionDirective"
FailsafeActive = "CommanderStatus.FailsafeActive"
StatusValid = "CommanderStatus.Valid"
RoutedRequestId = "MissionControlRequest.RequestId"
RoutedCommandId = "MissionControlRequest.CommandId"
RoutedAction = "MissionControlRequest.Action"
RoutedTargetNorth = "MissionControlRequest.TargetPositionNED(1)"
RoutedTargetEast = "MissionControlRequest.TargetPositionNED(2)"
RoutedTargetDown = "MissionControlRequest.TargetPositionNED(3)"
RoutedValid = "MissionControlRequest.Valid"
AckCommandId = "CommandAck.CommandId"
AckResult = "CommandAck.Result"
AckReasonCode = "CommandAck.ReasonCode"
AckValid = "CommandAck.Valid"
# --- end front-matter ---

Feature: Commander unified command gate and state review
  Commander is the single entry for GCS write commands: it audits each request, routes approved
  mode-level commands to MissionManager, returns one CommandAck per CommandId, and publishes its own
  status stream (CMD-001/005/006). Unlock follows the basic arm checks (CMD-003); "start mission" adds
  the seven takeoff gates (SYS-REQ-002..008).

Scenario: Unlock command with preconditions met arms the vehicle
  Given inputs
    * RequestCommandId = const(1)
    * RequestCommand = const(2)
    * RequestParam1 = const(0)
    * RequestParam2 = const(0)
    * RequestParam3 = const(0)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * MissionExecutionPhase = const(0)
    * MissionFaultCode = const(0)
    * MissionStatusValid = const(1)
    * LandingCompleted = const(0)
    * LinkConnected = const(1)
    * LinkValid = const(1)
    * EstimatorStatus = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * UnlockApproved: Armed == 1 when t > 0.6s
    * UnlockAcked: AckResult == 0 when t > 0.6s
    * AckCorrelatesCommand: AckCommandId == 1 when t > 0.6s
    * DecisionValid: StatusValid == 1 when t > 0.6s

Scenario: Lock command disarms the vehicle
  Given inputs
    * RequestCommandId = const(2)
    * RequestCommand = const(1)
    * RequestParam1 = const(0)
    * RequestParam2 = const(0)
    * RequestParam3 = const(0)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * MissionExecutionPhase = const(3)
    * MissionFaultCode = const(0)
    * MissionStatusValid = const(1)
    * LandingCompleted = const(1)
    * LinkConnected = const(1)
    * LinkValid = const(1)
    * EstimatorStatus = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * LockDisarms: Armed == 0 when t > 0.6s
    * LockAcked: AckResult == 0 when t > 0.6s

Scenario: Unlock with unhealthy estimator is rejected and stays disarmed
  Given inputs
    * RequestCommandId = const(3)
    * RequestCommand = const(2)
    * RequestParam1 = const(0)
    * RequestParam2 = const(0)
    * RequestParam3 = const(0)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * MissionExecutionPhase = const(0)
    * MissionFaultCode = const(0)
    * MissionStatusValid = const(1)
    * LandingCompleted = const(0)
    * LinkConnected = const(1)
    * LinkValid = const(1)
    * EstimatorStatus = const(2)
  When simulate for 1s in Normal mode
  Then outputs
    * UnlockRejected: Armed == 0 when t > 0.6s
    * UnlockRefused: AckResult == 1 when t > 0.6s

Scenario: Start mission command is audited and routed to MissionManager
  Given inputs
    * RequestCommandId = const(4)
    * RequestCommand = const(3)
    * RequestParam1 = const(0)
    * RequestParam2 = const(0)
    * RequestParam3 = const(0)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * MissionExecutionPhase = const(0)
    * MissionFaultCode = const(0)
    * MissionStatusValid = const(1)
    * LandingCompleted = const(0)
    * LinkConnected = const(1)
    * LinkValid = const(1)
    * EstimatorStatus = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * StartRoutedAsActionOne: RoutedAction == 1 when t > 0.6s
    * RouteCorrelatesCommand: RoutedCommandId == 4 when t > 0.6s
    * RouteIsValid: RoutedValid == 1 when t > 0.6s
    * MissionModeEntered: CurrentMode == 1 when t > 0.6s
    * StartAcked: AckResult == 0 when t > 0.6s

Scenario: Start mission with GCS link lost fails takeoff gate seven
  Given inputs
    * RequestCommandId = const(5)
    * RequestCommand = const(3)
    * RequestParam1 = const(0)
    * RequestParam2 = const(0)
    * RequestParam3 = const(0)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * MissionExecutionPhase = const(0)
    * MissionFaultCode = const(0)
    * MissionStatusValid = const(1)
    * LandingCompleted = const(0)
    * LinkConnected = step(1 -> 0 @ 1.2s)
    * LinkValid = const(1)
    * EstimatorStatus = const(1)
  When simulate for 2s in Normal mode
  Then outputs
    * StartRejectedAfterLinkLoss: AckResult == 1 when t > 1.5s
    * LinkGateFailed: AckReasonCode == 7 when t > 1.5s
    * NoRouteOnReject: RoutedValid == 0 when t > 1.5s

Scenario: Mode-level commands route to their actions
  Given inputs
    * RequestCommandId = step(6 -> 7 @ 1s)
    * RequestCommand = step(4 -> 5 @ 1s)
    * RequestParam1 = const(0)
    * RequestParam2 = const(0)
    * RequestParam3 = const(0)
    * RequestValid = step(0 -> 1 @ 0.5s)
    * MissionExecutionPhase = const(1)
    * MissionFaultCode = const(0)
    * MissionStatusValid = const(1)
    * LandingCompleted = const(0)
    * LinkConnected = const(1)
    * LinkValid = const(1)
    * EstimatorStatus = const(1)
  When simulate for 2s in Normal mode
  Then outputs
    * RtlRoutesToActionTwo: RoutedAction == 2 when t > 1.1s
    * LandRoutesToActionThree: RoutedAction == 3 when t > 1.6s
