function motor_cmd = allocateMotorCommands(thrust_N, torque_Nm, mixMatrix, ...
    motorMaxThrust_N, motorArmLength_m, motorMaxReactionTorque_Nm, ...
    motorMin, motorMax)
%ALLOCATEMOTORCOMMANDS Map total wrench in SI units to normalized commands.
% thrust_N is collective thrust [N], torque_Nm is [roll; pitch; yaw] [N*m].
% AirSim applies normalized command linearly to rotor max thrust and torque.

assert(numel(torque_Nm) == 3);

armProjection_m = motorArmLength_m / single(sqrt(2.0));
normalizedWrench = [ ...
    thrust_N / (single(4.0) * motorMaxThrust_N); ...
    torque_Nm(1) / (single(4.0) * armProjection_m * motorMaxThrust_N); ...
    torque_Nm(2) / (single(4.0) * armProjection_m * motorMaxThrust_N); ...
    torque_Nm(3) / (single(4.0) * motorMaxReactionTorque_Nm)];

motor_cmd = mixMatrix * normalizedWrench;
motor_cmd = max(motorMin, min(motorMax, motor_cmd));
motor_cmd = single(motor_cmd);
end
