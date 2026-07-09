function rosmsgOut = EscCmd(slBusIn, rosmsgOut)
%#codegen
%   Copyright 2021 The MathWorks, Inc.
    rosmsgOut.stamp = bus_conv_fcns.ros2.busToMsg.builtin_interfaces.Time(slBusIn.stamp,rosmsgOut.stamp(1));
    rosmsgOut.timestamp_sec = double(slBusIn.timestamp_sec);
    rosmsgOut.sequence = uint32(slBusIn.sequence);
    rosmsgOut.source_id = uint8(slBusIn.source_id);
    rosmsgOut.valid = logical(slBusIn.valid);
    rosmsgOut.motor_cmd = single(slBusIn.motor_cmd);
end
