function rosmsgOut = StateEst(slBusIn, rosmsgOut)
%#codegen
%   Copyright 2021 The MathWorks, Inc.
    rosmsgOut.stamp = bus_conv_fcns.ros2.busToMsg.builtin_interfaces.Time(slBusIn.stamp,rosmsgOut.stamp(1));
    rosmsgOut.timestamp_sec = double(slBusIn.timestamp_sec);
    rosmsgOut.sequence = uint32(slBusIn.sequence);
    rosmsgOut.source_id = uint8(slBusIn.source_id);
    rosmsgOut.valid = logical(slBusIn.valid);
    rosmsgOut.position_ned_m = single(slBusIn.position_ned_m);
    rosmsgOut.velocity_ned_mps = single(slBusIn.velocity_ned_mps);
    rosmsgOut.attitude_quat_wxyz = single(slBusIn.attitude_quat_wxyz);
    rosmsgOut.angular_rate_body_radps = single(slBusIn.angular_rate_body_radps);
    rosmsgOut.accel_body_mps2 = single(slBusIn.accel_body_mps2);
    rosmsgOut.gyro_bias_radps = single(slBusIn.gyro_bias_radps);
    rosmsgOut.accel_bias_mps2 = single(slBusIn.accel_bias_mps2);
    rosmsgOut.wind_ned_mps = single(slBusIn.wind_ned_mps);
    rosmsgOut.status = uint8(slBusIn.status);
end
