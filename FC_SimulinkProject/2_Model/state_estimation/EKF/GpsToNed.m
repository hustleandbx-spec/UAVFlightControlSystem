function posNed = GpsToNed(lat, lon, alt, originLat, originLon, refAlt, metersPerDeg)
%GPSTONED GPS 经纬高 → NED 位置（纯转换，无状态）
%   x_ned = (lat - originLat) * metersPerDeg              % 北向（米）
%   y_ned = (lon - originLon) * metersPerDeg * cosd(lat)  % 东向（米），纬圈大圆修正
%   z_ned = refAlt - alt                                   % 下向（米）
%
%   原点锁存状态由调用方（DelayEKFBlock）持有，本函数不保存任何状态；
%   metersPerDeg 为纬度每度米数（约 111320 m/deg，verify 口径）。
%#codegen

    x_ned = double(lat - originLat) * metersPerDeg;
    y_ned = double(lon - originLon) * metersPerDeg * cosd(double(lat));
    z_ned = double(refAlt - alt);

    posNed = single([x_ned; y_ned; z_ned]);
end
