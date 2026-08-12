# --- front-matter:toml ---
model = "D:\\Project\\UAVSingleFlightControl\\FC_SimulinkProject\\2_Model\\control\\UAV_FlightControl.slx"
component = "UAV_FlightControl/AttitudeRate"
[inputs]
AccN = "acc_desired(1)"
AccE = "acc_desired(2)"
AccD = "acc_desired(3)"
YawSP = "Yaw_SP"
Qw = "Attitude_quat(1)"
Qx = "Attitude_quat(2)"
Qy = "Attitude_quat(3)"
Qz = "Attitude_quat(4)"
RateP = "AngularRate_Body(1)"
RateQ = "AngularRate_Body(2)"
RateR = "AngularRate_Body(3)"
[outputs]
Torque = "torque_sp"
# --- end front-matter ---

Feature: Complete acceleration-to-attitude and body-rate controller
  The controller converts NED acceleration and yaw targets into body-axis torque.

Scenario: Level hover produces zero body torque
  Given inputs
    * AccN = const(0)
    * AccE = const(0)
    * AccD = const(0)
    * YawSP = const(0)
    * Qw = const(1)
    * Qx = const(0)
    * Qy = const(0)
    * Qz = const(0)
    * RateP = const(0)
    * RateQ = const(0)
    * RateR = const(0)
  When simulate for 200ms in Normal mode
  Then outputs
    * HoverTorqueZero: Torque == [-0.0001 .. 0.0001]

Scenario: Positive yaw request produces positive body yaw torque
  Given inputs
    * AccN = const(0)
    * AccE = const(0)
    * AccD = const(0)
    * YawSP = const(0.1)
    * Qw = const(1)
    * Qx = const(0)
    * Qy = const(0)
    * Qz = const(0)
    * RateP = const(0)
    * RateQ = const(0)
    * RateR = const(0)
  When simulate for 20ms in Normal mode
  Then outputs
    * YawTorqueBounded: Torque == [-2 .. 2]
