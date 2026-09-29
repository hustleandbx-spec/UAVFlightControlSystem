function [x_c, P_c, accepted] = EkfMeasUpdate(x_p, P_p, z, h, H, R, gate2)
%EkfMeasUpdate  通用 EKF 测量更新核（序贯更新单步，三路传感器共用）
%
% 唯一的 KF 数学骨架：新息计算、卡尔曼增益、流形 retraction、Joseph 协方差。
% 所有传感器（GPS/气压/磁力计）的观测模型只负责给出 z/h/H/R，不重复写 KF 代数。
%
% 野值门限（马氏距离，χ² 门控）：
%   d2 = innov' * inv(S) * innov，S = H*P*H' + R；
%   d2 > gate2 时直接保持 x/P（accepted=false），避免坏采样污染滤波器。
%   gate2 取 χ²(dof, 0.999) 量级（dof = 测量维数）：baro 1D≈12，mag 3D≈16，gps 6D≈25。
%
% 误差态 retraction 为左乘流形：q_c = dq ⊗ q_p，dq = exp(delta_theta/2)，
% 与 FusePredict / 既有全部更新函数一致。
%
% 输入：
%   x_p   16x1 名义状态 [p(3); v(3); q(4,wxyz); ba(3); bg(3)]
%   P_p   15x15 误差态协方差 [dp; dv; dtheta; dba; dbg]
%   z     mx1 测量值；h  mx1 预测值 h(x_p)；H mx15 雅可比；R mxm 测量噪声；
%   gate2 马氏门限平方（<=0 表示不门控）
% 输出：
%   x_c/P_c 更新后状态/协方差；accepted=true 表示实际融合，false 表示被门限拒绝。
%#codegen
    innov = z(:) - h(:);
    S = H * P_p * H' + R;
    d2 = innov' / S * innov;

    x_c = x_p;
    P_c = P_p;
    accepted = false;

    if gate2 <= 0 || d2 <= gate2
        K = (P_p * H') / S;
        dx = K * innov;

        % 流形 retraction（唯一实现，三路传感器共用此段）
        x_c(1:3)   = x_p(1:3)   + dx(1:3);
        x_c(4:6)   = x_p(4:6)   + dx(4:6);
        dq = quat_exp(dx(7:9), 1.0);
        x_c(7:10)  = quat_mult(dq, x_p(7:10));
        x_c(7:10)  = quat_normalize(x_c(7:10));
        x_c(11:13) = x_p(11:13) + dx(10:12);
        x_c(14:16) = x_p(14:16) + dx(13:15);

        % Joseph 形式（保正定）
        I = eye(15, 'like', x_p);
        P_c = (I - K*H) * P_p * (I - K*H)' + K * R * K';
        P_c = (P_c + P_c') / 2;
        accepted = true;
    end
end
