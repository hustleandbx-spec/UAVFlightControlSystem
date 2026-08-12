# --- front-matter:toml ---
# 本文件验证 Navigator 段轨迹的停点减速剖面（NAV-003）：FC 只跟踪速度参考，
# 速度剖面含停点减速；Navigator 判据无感知、无"到达完成"概念（DEC-062），
# 到达/推进判据归 MM。字段对齐 A2 契约 #1/#2。
model = "Navigator.slx"
[inputs]
ContractId = "NavigationContract.ContractId"
PlanId = "NavigationContract.PlanId"
SegmentIndex = "NavigationContract.SegmentIndex"
StartValid = "NavigationContract.StartValid"
StartNorth = "NavigationContract.StartPositionNED(1)"
StartEast = "NavigationContract.StartPositionNED(2)"
StartDown = "NavigationContract.StartPositionNED(3)"
PassCount = "NavigationContract.PassCount"
EndNorth = "NavigationContract.EndPositionNED(1)"
EndEast = "NavigationContract.EndPositionNED(2)"
EndDown = "NavigationContract.EndPositionNED(3)"
CruiseSpeed = "NavigationContract.CruiseSpeed"
MaxAcceleration = "NavigationContract.MaxAcceleration"
MaxJerk = "NavigationContract.MaxJerk"
TargetYaw = "NavigationContract.TargetYaw"
ContractValid = "NavigationContract.Valid"
PositionNorth = "StateEstBus.Position_NED(1)"
PositionEast = "StateEstBus.Position_NED(2)"
PositionDown = "StateEstBus.Position_NED(3)"
VelocityNorth = "StateEstBus.Velocity_NED(1)"
VelocityEast = "StateEstBus.Velocity_NED(2)"
VelocityDown = "StateEstBus.Velocity_NED(3)"
EstimatorStatus = "StateEstBus.Status"
[outputs]
PosNorthSP = "TrajectorySetpoint.Position_NED_SP(1)"
PosDownSP = "TrajectorySetpoint.Position_NED_SP(3)"
VelNorthSP = "TrajectorySetpoint.Velocity_NED_SP(1)"
VelEastSP = "TrajectorySetpoint.Velocity_NED_SP(2)"
VelDownSP = "TrajectorySetpoint.Velocity_NED_SP(3)"
YawSP = "TrajectorySetpoint.Yaw_SP"
TrajectoryValid = "TrajectorySetpoint.Valid"
StatusContractId = "NavigationStatus.ContractId"
GuidanceState = "NavigationStatus.GuidanceState"
StatusValid = "NavigationStatus.Valid"
# --- end front-matter ---

Feature: Navigator segment stop-and-go velocity profile
  The segment reference tracks the end point and decelerates toward it; the design terminal state
  (reached + settled) yields a hover hold reference (NAV-003/004).

Scenario: Distant stop produces a bounded cruise command toward the end
  Given inputs
    * ContractId = const(101)
    * PlanId = const(200)
    * SegmentIndex = const(0)
    * StartValid = const(1)
    * StartNorth = const(0)
    * StartEast = const(0)
    * StartDown = const(-5)
    * PassCount = const(0)
    * EndNorth = const(10)
    * EndEast = const(0)
    * EndDown = const(-5)
    * CruiseSpeed = const(2)
    * MaxAcceleration = const(1)
    * MaxJerk = const(2)
    * TargetYaw = const(0)
    * ContractValid = const(1)
    * PositionNorth = const(0)
    * PositionEast = const(0)
    * PositionDown = const(-5)
    * VelocityNorth = const(0)
    * VelocityEast = const(0)
    * VelocityDown = const(0)
    * EstimatorStatus = const(1)
  When simulate for 1s in Normal mode
  Then outputs
    * CommandPointsTowardEnd: VelNorthSP > 0 when t > 0.5s
    * CruiseSpeedBounded: VelNorthSP <= 2 when t > 0.5s
    * NavigationRemainsValid: StatusValid == 1 when t > 0.5s
    * SegmentEvaluationActive: GuidanceState == 1 when t > 0.5s

Scenario: Approaching the stop decelerates the velocity reference
  Given inputs
    * ContractId = const(102)
    * PlanId = const(201)
    * SegmentIndex = const(0)
    * StartValid = const(1)
    * StartNorth = const(1)
    * StartEast = const(0)
    * StartDown = const(-5)
    * PassCount = const(0)
    * EndNorth = const(0)
    * EndEast = const(0)
    * EndDown = const(-5)
    * CruiseSpeed = const(2)
    * MaxAcceleration = const(1)
    * MaxJerk = const(2)
    * TargetYaw = const(0)
    * ContractValid = const(1)
    * PositionNorth = const(1)
    * PositionEast = const(0)
    * PositionDown = const(-5)
    * VelocityNorth = const(0.5)
    * VelocityEast = const(0)
    * VelocityDown = const(0)
    * EstimatorStatus = const(1)
  When simulate for 2s in Normal mode
  Then outputs
    * ReferenceDeceleratesNearStop: VelNorthSP < 2 when t > 1s
    * SegmentEvaluationActive: GuidanceState == 1 when t > 1s

Scenario: Settled at the stop transitions to hover hold with zero velocity
  Given inputs
    * ContractId = const(103)
    * PlanId = const(202)
    * SegmentIndex = const(0)
    * StartValid = const(1)
    * StartNorth = const(0)
    * StartEast = const(0)
    * StartDown = const(-5)
    * PassCount = const(0)
    * EndNorth = const(0)
    * EndEast = const(0)
    * EndDown = const(-5)
    * CruiseSpeed = const(2)
    * MaxAcceleration = const(1)
    * MaxJerk = const(2)
    * TargetYaw = const(0)
    * ContractValid = const(1)
    * PositionNorth = const(0)
    * PositionEast = const(0)
    * PositionDown = const(-5)
    * VelocityNorth = const(0)
    * VelocityEast = const(0)
    * VelocityDown = const(0)
    * EstimatorStatus = const(1)
  When simulate for 2s in Normal mode
  Then outputs
    * HoldStateEntered: GuidanceState == 2 when t > 1s
    * HoldsEndPosition: PosNorthSP == 0 when t > 1s
    * HoldVelocityZero: VelNorthSP == 0 when t > 1s
    * NormalFlowRemainsValid: StatusValid == 1 when t > 1s
