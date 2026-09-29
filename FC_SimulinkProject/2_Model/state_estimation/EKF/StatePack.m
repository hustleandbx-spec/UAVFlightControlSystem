function [est, snapshot, attitude, health, diagnostic] = StatePack(x, accelBody, angRate, imuSeq, imuTime, imuValid, ready, gpsLocked, magHeadingValid, baroValid)
%StatePack 从输出状态生成五路契约输出（与 EKF.slx StatePack 同构）
%
% 有效性语义（修正 P1-1：不再用 GPS 融合脉冲驱动）：
%   - Valid 由 imuValid && ready 驱动：融合窗口建立后持续为真；
%   - PositionEstimateValid = ready && gpsLocked（GPS 已锁存原点且融合就绪）；
%   - SensorsHealthy 用各传感器持续可用性（gpsLocked 为锁存态，非融合脉冲）。
% 状态布局 16 维：[p(3); v(3); q(4); ba(3); bg(3)]。
%#codegen

    pos     = single(x(1:3));
    vel     = single(x(4:6));
    quat    = single(x(7:10));
    valid   = imuValid && ready;

    est.Position_NED     = pos;
    est.Velocity_NED     = vel;
    est.Attitude_quat    = quat;
    est.AngularRate_Body = angRate;
    est.Sequence         = imuSeq;
    est.Timestamp        = imuTime;
    est.Valid            = valid;

    snapshot.Position_NED = pos;
    snapshot.Velocity_NED = vel;
    snapshot.Sequence     = imuSeq;
    snapshot.Timestamp    = imuTime;
    snapshot.Valid        = valid;

    attitude.Attitude_quat    = quat;
    attitude.AngularRate_Body = angRate;
    attitude.Sequence         = imuSeq;
    attitude.Timestamp        = imuTime;
    attitude.Valid            = valid;

    health.PositionEstimateValid = ready && gpsLocked;
    health.HeadingEstimateValid  = magHeadingValid;
    health.SensorsHealthy        = imuValid && gpsLocked && magHeadingValid && baroValid;
    health.EstimatorHealthy      = valid;
    health.Sequence              = imuSeq;
    health.Timestamp             = imuTime;
    health.Valid                 = valid;

    diagnostic.Accel_Body = single(accelBody);
    diagnostic.GyroBias   = single(x(14:16));
    diagnostic.AccelBias  = single(x(11:13));
    diagnostic.Wind_NED   = single([0;0;0]);
    diagnostic.Sequence   = imuSeq;
    diagnostic.Timestamp  = imuTime;
    diagnostic.Valid      = valid;
end
