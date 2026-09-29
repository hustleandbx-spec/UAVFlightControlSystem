function [x_p, P_p] = FusePredict(x_last, P_last, accel, gyro, dt, g_n, noise_cfg)
%FusePredict 融合状态预测（延迟样本推进）
%
% 状态布局（名义状态 16 维）：
%   1:3 Position_NED / 4:6 Velocity_NED / 7:10 Attitude_quat [w;x;y;z]
%   11:13 AccelBias / 14:16 GyroBias
%#codegen

    sigma_acc = noise_cfg(1);
    sigma_gyr = noise_cfg(2);
    sigma_ba  = noise_cfg(3);
    sigma_bg  = noise_cfg(4);

    Q_c = diag([sigma_acc*ones(1,3,'like',sigma_acc), sigma_gyr*ones(1,3,'like',sigma_acc), sigma_ba*ones(1,3,'like',sigma_acc), sigma_bg*ones(1,3,'like',sigma_acc)]);

    % --- 1. 状态提取与偏置补偿 ---
    p = x_last(1:3); v = x_last(4:6); q = x_last(7:10);
    ba = x_last(11:13); bg = x_last(14:16);

    accel_corr = accel - ba;
    omega_corr = gyro - bg;

    % --- 2. 状态预测 (非线性动力学) ---
    dq = quat_exp(omega_corr, dt);
    q_p = quat_mult(q, dq);
    q_p = quat_normalize(q_p);

    C_bn = quat2rotm(q);
    a_n = C_bn * accel_corr + g_n;
    v_p = v + a_n * dt;
    p_p = p + v * dt + 0.5 * a_n * dt^2;

    x_p = [p_p; v_p; q_p; ba; bg];

    % --- 3. 协方差预测 ---
    % 连续时间状态转移 Jacobian（线性化全状态动力学）
    F_c = zeros(15, 15, 'like', x_last);
    a_sk = skew_sym(accel_corr);
    w_sk = skew_sym(omega_corr);

    F_c(1:3, 4:6)   = eye(3, 'like', x_last);
    F_c(4:6, 7:9)   = -C_bn * a_sk;
    F_c(4:6, 10:12) = -C_bn;
    F_c(7:9, 7:9)   = -w_sk;
    F_c(7:9, 13:15) = -eye(3, 'like', x_last);

    F = eye(15, 'like', x_last) + F_c * dt;

    G = zeros(15, 12, 'like', x_last);
    G(4:6, 1:3)    = -C_bn;
    G(7:9, 4:6)    = -eye(3, 'like', x_last);
    G(10:12, 7:9)  = eye(3, 'like', x_last);
    G(13:15, 10:12) = eye(3, 'like', x_last);

    Q_d = G * Q_c * G' * dt;
    P_p = F * P_last * F' + Q_d;
    P_p = (P_p + P_p') / 2;
end
