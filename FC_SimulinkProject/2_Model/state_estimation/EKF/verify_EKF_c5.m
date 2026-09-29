% verify_EKF_c5.m — C5 磁收敛验证（类级）：XInit 初值航向 +20°，磁力计端到端收敛
% 通过 Nontunable 属性在解释环境赋值覆盖初值（Simulink 块参数编译期锁死，
% 故类级验证）；位置/速度/航向断言同 verify_EKF_model。
rootDir = 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject';
cd(rootDir);
try
    openProject(fullfile(rootDir, 'FC_SimulinkProject.prj'));
catch e
    disp(['openProject: ' e.message]);
    run(fullfile(rootDir, 'start.m'));
end

% --- 初值航向 +20° 的四元数 XInit ---
th = deg2rad(20);
x0 = single([0;0;0; 0;0;0; cos(th/2);0;0;sin(th/2); 0;0;0; 0;0;0]);
obj = DelayEKFBlock();
obj.XInit = x0;

% --- 2s @1kHz 场景：先加速后匀速，GPS 5Hz / Baro 10Hz / Mag 10Hz ---
metersPerDeg = 111320.0;
lat0 = 30.0; lon0 = 104.0;
mNed = single([33.9322; -1.4444; 38.2691]);
pTrue = @(tv) min(tv,1).^2 + 2*max(tv-1,0);
vTrue = @(tv) 2*min(tv,1);

lastEst = [];
for i = 1:2000
    tNow = (i-1)*1e-3;
    aNow = single(2*(tNow < 1));
    imu = struct('Accel', single([aNow; 0; -9.81]), 'Gyro', single([0;0;0]), ...
                 'Sequence', uint32(i), 'Timestamp', double(tNow), 'IsNew', true, 'Valid', true);
    gps = struct('Lat', single(lat0 + pTrue(tNow)/metersPerDeg), 'Lon', single(lon0), ...
                 'Alt', single(0), 'Velocity', single([vTrue(tNow);0;0]), ...
                 'FixType', uint8(3), 'NumSat', uint8(12), 'Eph', single(0.01), 'Epv', single(0.01), ...
                 'Sequence', uint32(i), 'Timestamp', double(tNow), 'IsNew', mod(i,200)==1, 'Valid', true);
    baro = struct('Pressure', single(101325), 'Temperature', single(25), ...
                  'Sequence', uint32(i), 'Timestamp', double(tNow), 'IsNew', mod(i,100)==1, 'Valid', true);
    mag = struct('Mag_Body', mNed.', 'Heading_Mag', single(0), 'HeadingValid', true, 'Declination', single(0), ...
                 'Sequence', uint32(i), 'Timestamp', double(tNow), 'IsNew', mod(i,100)==1, 'Valid', true);
    [est, ~, ~, ~, ~] = obj.step(imu, gps, mag, baro);
    lastEst = est;
end

q = lastEst.Attitude_quat;
yawEnd = atan2(2*(q(1)*q(4) + q(2)*q(3)), 1 - 2*(q(3)^2 + q(4)^2));
pN = lastEst.Position_NED;  vN = lastEst.Velocity_NED;
pTrueEnd = pTrue(2.0);  vTrueEnd = vTrue(2.0);
fprintf('  末态 yaw = %.3f deg（阈值 3）\n', rad2deg(abs(yawEnd)));
fprintf('  末态位置误差 = %.3f m（阈值 0.15）\n', abs(pN(1)-pTrueEnd));
fprintf('  末态速度误差 = %.3f m/s（阈值 0.3）\n', abs(vN(1)-vTrueEnd));
fprintf('  末态 Valid = %d\n', lastEst.Valid);

ok = abs(yawEnd) < deg2rad(3) && abs(pN(1)-pTrueEnd) < 0.15 && abs(vN(1)-vTrueEnd) < 0.3 && lastEst.Valid;
fprintf('\n=== C5 磁收敛验证 %s ===\n', ternary(ok, '通过：+20° 初值航向被磁力计拉回 ✓', '未通过 ✗'));
if ~ok, error('verify_EKF_c5: 未通过断言'); end
fprintf('== verify_EKF_c5 完成 ==\n');

function s = ternary(c, a, b)
    if c, s = a; else, s = b; end
end
