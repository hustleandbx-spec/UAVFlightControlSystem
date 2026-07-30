# The former MATLAB Desktop / BlockingStepResult participant was retired.
# The strict-lockstep entry will be restored only after the generated
# FlightCore ROS 2 node and ROS 2 time-model-stepping path are deployed.

$ErrorActionPreference = 'Stop'

throw @'
旧的 MATLAB Desktop / StepResult 联合仿真入口已移除。
当前模型使用原生 IMU/GPS Subscribe；请勿恢复旧 Desktop participant。
generated FlightCore ROS 2 node 的正式 WSL 入口尚未在此脚本中启用。
'@
