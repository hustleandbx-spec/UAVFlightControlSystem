function slBusOut = Imu(msgIn, slBusOut, varargin)
%#codegen
%   Copyright 2021-2022 The MathWorks, Inc.
    currentlength = length(slBusOut.stamp);
    for iter=1:currentlength
        slBusOut.stamp(iter) = bus_conv_fcns.ros2.msgToBus.builtin_interfaces.Time(msgIn.stamp(iter),slBusOut(1).stamp(iter),varargin{:});
    end
    slBusOut.stamp = bus_conv_fcns.ros2.msgToBus.builtin_interfaces.Time(msgIn.stamp,slBusOut(1).stamp,varargin{:});
    slBusOut.timestamp_sec = double(msgIn.timestamp_sec);
    slBusOut.sequence = uint32(msgIn.sequence);
    slBusOut.source_id = uint8(msgIn.source_id);
    slBusOut.valid = logical(msgIn.valid);
                    currentlength = length(slBusOut.accel_mps2);
                    slBusOut.accel_mps2 = single(msgIn.accel_mps2(1:currentlength));
                    currentlength = length(slBusOut.gyro_radps);
                    slBusOut.gyro_radps = single(msgIn.gyro_radps(1:currentlength));
end
