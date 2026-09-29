% verify_EKF_fuse.m — 正式 Fuse 函数单元测试：磁闭环数学 + 野值门限
% （提炼自旧 verify_INS_EKF_PX4.m 的 B3/B4，仅依赖正式代码 FuseUpdateMag/EkfMeasUpdate）
rootDir = 'D:\Project\UAVSingleFlightControl\FC_SimulinkProject';
cd(rootDir);
try
    openProject(fullfile(rootDir, 'FC_SimulinkProject.prj'));
catch e
    disp(['openProject: ' e.message]);
    run(fullfile(rootDir, 'start.m'));
end

m_ned  = single([33.9322; -1.4444; 38.2691]);   % WMM2025 成都参考场
rMag   = single(1e-2);
gateMag = single(-1);      % B3 不门控（纯测 KF 收敛数学）；野值门限见 B4
yawOfQ = @(q) atan2(2*(q(1)*q(4) + q(2)*q(3)), 1 - 2*(q(3)^2 + q(4)^2));

yaw0 = deg2rad(20);
q_true = single([1;0;0;0]);                       % 真值水平 yaw=0
q_est  = single([cos(yaw0/2);0;0;sin(yaw0/2)]);   % 估计 yaw=+20 度
x_p = single([0;0;0; 0;0;0; q_est; 0;0;0; 0;0;0]);
P_p = single(diag([0.25 0.25 0.25 0.04 0.04 0.04 0.01 0.01 0.01 2.5e-3 2.5e-3 2.5e-3 1e-4 1e-4 1e-4]));
magBody = single(quat2rotm(q_true).' * m_ned);    % 体轴磁测（真值姿态）

% --- B3a 单次磁更新：+20° 误差被缩减 30% 以上且方向正确 ---
[x_c, ~] = FuseUpdateMag(x_p, P_p, magBody, m_ned, rMag, gateMag);
yawBefore = yawOfQ(q_est); yawAfter = yawOfQ(x_c(7:10));
fprintf('  B3a 单次更新 yaw %.2f deg -> %.2f deg（真值 0）\n', rad2deg(yawBefore), rad2deg(yawAfter));
assert(abs(yawAfter) < abs(yawBefore)*0.7, 'B3a: 单次更新后航向误差未缩减 30%% 以上');
assert(yawAfter * yawBefore >= 0, 'B3a: 单次更新方向错误');

% --- B3b 纯磁闭环 2000 拍：创新收敛、数值稳定 ---
x = x_p; P = P_p; inov1 = 0; inovN = 0;
for k = 1:2000
    h = quat2rotm(x(7:10)).' * m_ned;
    if k == 1, inov1 = norm(magBody - h); end
    [x, P] = FuseUpdateMag(x, P, magBody, m_ned, rMag, gateMag);
end
inovN = norm(magBody - quat2rotm(x(7:10)).' * m_ned);
yawFinal = yawOfQ(x(7:10));
fprintf('  B3b 纯磁闭环 2000 拍 yaw=%.2f deg, |inov| %.4f -> %.4f uT\n', ...
    rad2deg(yawFinal), inov1, inovN);
assert(abs(yawFinal) < deg2rad(30), 'B3b: 纯磁闭环发散');
assert(inovN < inov1 * 0.5, 'B3b: 磁创新未收敛');

% --- B4 野值门限：坏磁测（10x 场强）必须被 χ² 门限拒绝，状态不动 ---
magBad = single(10.0 * m_ned);
hEst   = quat2rotm(q_est).' * m_ned;
[x_g, P_g, accepted] = EkfMeasUpdate(x_p, P_p, magBad, hEst, ...
    zeros(3,15,'like',x_p), rMag*eye(3,'like',rMag), single(16.0));
fprintf('  B4 野值磁测 accepted=%d, yaw=%.3f deg（应保持 20.00 不动）\n', ...
    accepted, rad2deg(yawOfQ(x_g(7:10))));
assert(accepted == false, 'B4: 野值未被门限拒绝');
assert(norm(x_g - x_p) < 1e-6, 'B4: 野值污染了状态');

fprintf('\n=== Fuse 单元测试 全部通过 ✓ ===\n');
fprintf('== verify_EKF_fuse 完成 ==\n');
