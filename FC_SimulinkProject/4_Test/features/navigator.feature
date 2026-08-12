# --- front-matter:toml ---
# 本文件验证 Navigator 消费 NavigationContractBus v2（运动学合同）并输出
# TrajectorySetpointBus（连续轨迹参考）+ NavigationStatusBus（制导状态流）。
# 字段对齐 A2 契约 #1/#2/#4（DEC-085/086），到达判据归 MM，Navigator 判据无感知（DEC-062）。
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
PosEastSP = "TrajectorySetpoint.Position_NED_SP(2)"
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

Feature: Navigator consumes a v2 kinematic contract and produces a continuous trajectory reference
  Navigator is the only trajectory generator (NAV-001): it plans the whole segment from the current state
  snapshot and evaluates the segment reference deterministically, publishing guidance state without any
  arrival judgment (NAV-002/005, DEC-062). In the normal flow its output stays valid (worst case = hover hold, DEC-086).

Scenario: Valid distant segment starts evaluation with correlation intact
  Given inputs
    * ContractId = const(41)
    * PlanId = const(100)
    * SegmentIndex = const(0)
    * StartValid = const(1)
    * StartNorth = const(0)
    * StartEast = const(0)
    * StartDown = const(-2)
    * PassCount = const(0)
    * EndNorth = const(10)
    * EndEast = const(0)
    * EndDown = const(-5)
    * CruiseSpeed = const(2)
    * MaxAcceleration = const(1)
    * MaxJerk = const(2)
    * TargetYaw = const(0.75)
    * ContractValid = const(1)
    * PositionNorth = const(0)
    * PositionEast = const(0)
    * PositionDown = const(-2)
    * VelocityNorth = const(0)
    * VelocityEast = const(0)
    * VelocityDown = const(0)
    * EstimatorStatus = const(1)
  When simulate for 5ms in Normal mode
  Then outputs
    * SegmentEvaluationActive: GuidanceState == 1
    * ContractCorrelationPreserved: StatusContractId == 41
    * TrajectoryIsValid: TrajectoryValid == 1
    * StatusIsValid: StatusValid == 1
    * YawPreserved: YawSP == 0.75
    * TrajectoryAnchoredAtStart: PosNorthSP == 0

Scenario: Segment end reached and settled transitions to hover hold
  Given inputs
    * ContractId = const(42)
    * PlanId = const(101)
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
    * HoldsSegmentEndPosition: PosNorthSP == 0 when t > 1s
    * HoldsEndAltitude: PosDownSP == -5 when t > 1s
    * HoldVelocityIsZero: VelNorthSP == 0 when t > 1s
    * NormalFlowRemainsValid: StatusValid == 1 when t > 1s

Scenario: Invalid contract suppresses segment evaluation but keeps status valid
  Given inputs
    * ContractId = const(43)
    * PlanId = const(102)
    * SegmentIndex = const(0)
    * StartValid = const(0)
    * StartNorth = const(0)
    * StartEast = const(0)
    * StartDown = const(-2)
    * PassCount = const(0)
    * EndNorth = const(10)
    * EndEast = const(0)
    * EndDown = const(-5)
    * CruiseSpeed = const(2)
    * MaxAcceleration = const(1)
    * MaxJerk = const(2)
    * TargetYaw = const(0)
    * ContractValid = const(0)
    * PositionNorth = const(0)
    * PositionEast = const(0)
    * PositionDown = const(-2)
    * VelocityNorth = const(0)
    * VelocityEast = const(0)
    * VelocityDown = const(0)
    * EstimatorStatus = const(1)
  When simulate for 5ms in Normal mode
  Then outputs
    * WaitingForContract: GuidanceState == 0
    * InvalidContractSuppressesSegment: TrajectoryValid == 0
    * NormalFlowRemainsValid: StatusValid == 1

Scenario: Unstable estimator suppresses trajectory output
  Given inputs
    * ContractId = const(44)
    * PlanId = const(103)
    * SegmentIndex = const(0)
    * StartValid = const(1)
    * StartNorth = const(0)
    * StartEast = const(0)
    * StartDown = const(-2)
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
    * PositionDown = const(-2)
    * VelocityNorth = const(0)
    * VelocityEast = const(0)
    * VelocityDown = const(0)
    * EstimatorStatus = const(0)
  When simulate for 5ms in Normal mode
  Then outputs
    * UnstableEstimatorSuppressesTrajectory: TrajectoryValid == 0
