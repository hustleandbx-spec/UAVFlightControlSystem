function slBusOut = Gps(msgIn, slBusOut, varargin)
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
    slBusOut.lat_deg = single(msgIn.lat_deg);
    slBusOut.lon_deg = single(msgIn.lon_deg);
    slBusOut.alt_m = single(msgIn.alt_m);
                    currentlength = length(slBusOut.velocity_ned_mps);
                    slBusOut.velocity_ned_mps = single(msgIn.velocity_ned_mps(1:currentlength));
end
