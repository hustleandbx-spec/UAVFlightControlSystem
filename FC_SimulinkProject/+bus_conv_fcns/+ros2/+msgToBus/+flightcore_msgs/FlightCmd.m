function slBusOut = FlightCmd(msgIn, slBusOut, varargin)
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
    slBusOut.mode = uint8(msgIn.mode);
                    currentlength = length(slBusOut.position_ned_sp_m);
                    slBusOut.position_ned_sp_m = single(msgIn.position_ned_sp_m(1:currentlength));
                    currentlength = length(slBusOut.velocity_ned_sp_mps);
                    slBusOut.velocity_ned_sp_mps = single(msgIn.velocity_ned_sp_mps(1:currentlength));
    slBusOut.yaw_sp_rad = single(msgIn.yaw_sp_rad);
end
