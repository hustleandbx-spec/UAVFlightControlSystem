% verify_EKF_model.m — 正式模型 EKF.slx 端到端仿真验证（C1-C4 口径）
% 场景：2s @1kHz，先匀加速(a=2)后匀速(v=2)，GPS 5Hz / Baro 10Hz / Mag 10Hz。
% 断言：稳态段[1.5,2.0]s est.Valid/健康位持续为真、时间戳单调、
%       位置误差<0.15m、速度偏差<0.3m/s、航向漂移<3°。
rootDir = 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject';
cd(rootDir);
try
    openProject(fullfile(rootDir, 'FC_SimulinkProject.prj'));
catch e
    disp(['openProject: ' e.message]);
    run(fullfile(rootDir, 'start.m'));
end

Mdl = 'EKF';
load_system(fullfile(rootDir, '2_Model', 'state_estimation', 'EKF', [Mdl '.slx']));

% --- 构造总线外部输入（structure-with-timeseries，字段=叶子名）---
t = (0:0.001:2.0)'; n = numel(t);
dT = double(t); sT = single(t);
ts = @(v) timeseries(v, t);

aNow = single(2 * (t < 1));
imu = struct();
imu.Accel    = ts(single([aNow, zeros(n,1), -9.81*ones(n,1)]));
imu.Gyro     = ts(zeros(n,3,'single'));
imu.Sequence = ts(uint32(1:n)');
imu.Timestamp= ts(dT);
imu.IsNew    = ts(true(n,1));
imu.Valid    = ts(true(n,1));

lat0 = 30.0; lon0 = 104.0; metersPerDeg = 111320.0;
pTrue = @(tv) min(tv,1).^2 + 2*max(tv-1,0);
vTrue = @(tv) 2*min(tv,1);
tMeas = max(dT - 0.02, 0);
gpsNew = false(n,1); gpsNew(1:200:end) = true;
gps = struct();
gps.Lat       = ts(single(lat0 + pTrue(tMeas)/metersPerDeg));
gps.Lon       = ts(single(repmat(lon0, n, 1)));
gps.Alt       = ts(single(zeros(n,1)));
gps.Velocity  = ts(single([vTrue(dT), zeros(n,1), zeros(n,1)]));
gps.FixType   = ts(repmat(uint8(3),n,1));
gps.NumSat    = ts(repmat(uint8(12),n,1));
gps.Eph       = ts(repmat(single(0.01),n,1));
gps.Epv       = ts(repmat(single(0.01),n,1));
gps.Sequence  = ts(uint32(1:n)');
gps.Timestamp = ts(dT);
gps.IsNew     = ts(gpsNew);
gps.Valid     = ts(true(n,1));

baroNew = false(n,1); baroNew(1:100:end) = true;
baro = struct();
baro.Pressure   = ts(repmat(single(101325),n,1));
baro.Temperature= ts(repmat(single(25),n,1));
baro.Sequence   = ts(uint32(1:n)');
baro.Timestamp  = ts(dT);
baro.IsNew      = ts(baroNew);
baro.Valid      = ts(true(n,1));

mNed = [33.9322; -1.4444; 38.2691];
magNew = false(n,1); magNew(1:100:end) = true;
mag = struct();
mag.Mag_Body    = ts(repmat(single(mNed.'), n, 1));
mag.Heading_Mag = ts(repmat(single(0),n,1));
mag.HeadingValid= ts(true(n,1));
mag.Declination = ts(repmat(single(0),n,1));
mag.Sequence    = ts(uint32(1:n)');
mag.Timestamp   = ts(dT);
mag.IsNew       = ts(magNew);
mag.Valid       = ts(true(n,1));

assignin('base','imu_in',imu);  assignin('base','gps_in',gps);
assignin('base','baro_in',baro); assignin('base','mag_in',mag);

fprintf('== 仿真 EKF.slx 2s ==\n');
out = sim(Mdl, 'LoadExternalInput', 'on', ...
          'ExternalInput', 'imu_in, gps_in, baro_in, mag_in', ...
          'StopTime', '2.0');
fprintf('仿真完成，输出通道数 = %d\n', numel(out.yout));

est    = out.yout{1}.Values;   % StateEstBus
snap   = out.yout{2}.Values;   % StateSnapshot
att    = out.yout{3}.Values;   % StateAttitudeSnapshot
health = out.yout{4}.Values;   % StateEstHealth

ok = true;
valid = est.Valid; pos = est.Position_NED; vel = est.Velocity_NED;
hValid = health.Valid; hPosEst = health.PositionEstimateValid; hSensors = health.SensorsHealthy;
idx = valid.Time >= 1.5;

vVals = squeeze(valid.Data(idx)); pVals = squeeze(hPosEst.Data(idx)); sVals = squeeze(hSensors.Data(idx));
fprintf('  est.Valid 稳态真比例: %.1f%%\n', 100*mean(vVals));
fprintf('  PositionEstimateValid: %.1f%%  SensorsHealthy: %.1f%%\n', 100*mean(pVals), 100*mean(sVals));
ok = ok && all(vVals) && all(pVals) && all(sVals);

tSt = squeeze(est.Timestamp.Data);
fprintf('  est.Timestamp 单调: %s\n', mat2str(all(diff(tSt(idx)) >= -1e-9)));
ok = ok && all(diff(tSt(idx)) >= -1e-9);

pN = squeeze(pos.Data(:,1)); tN = valid.Time;
posErr = abs(pN(idx) - pTrue(tN(idx)));
fprintf('  位置误差 max = %.3f m（阈值 0.15）\n', max(posErr));
ok = ok && max(posErr) < 0.15;

vN = squeeze(vel.Data(:,1));
fprintf('  速度偏差 max = %.3f m/s（阈值 0.3）\n', max(abs(vN(idx)-2)));
ok = ok && max(abs(vN(idx)-2)) < 0.3;

attN = squeeze(est.Attitude_quat.Data);
yawN = atan2(2*(attN(:,1).*attN(:,4) + attN(:,2).*attN(:,3)), ...
             1 - 2*(attN(:,3).^2 + attN(:,4).^2));
fprintf('  稳态 max|yaw| = %.3f deg（阈值 3）\n', rad2deg(max(abs(yawN(idx)))));
ok = ok && max(abs(yawN(idx))) < deg2rad(3);

fprintf('  snapshot.Valid 稳态真比例: %.1f%%\n', 100*mean(squeeze(snap.Valid.Data(idx))));
fprintf('  attitude.Valid 稳态真比例: %.1f%%\n', 100*mean(squeeze(att.Valid.Data(idx))));
ok = ok && all(squeeze(snap.Valid.Data(idx))) && all(squeeze(att.Valid.Data(idx)));

fprintf('\n=== EKF.slx 端到端验证 %s ===\n', ternary(ok, '全部通过 ✓', '存在未通过项 ✗'));
if ~ok, error('verify_EKF_model: 存在未通过断言'); end
fprintf('== verify_EKF_model 完成 ==\n');

function s = ternary(c, a, b)
    if c, s = a; else, s = b; end
end
