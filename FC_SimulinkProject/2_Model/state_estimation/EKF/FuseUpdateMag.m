function [x_c, P_c] = FuseUpdateMag(x_p, P_p, magBody, magNed, rMag, gate2)
%FuseUpdateMag 磁力计观测模型层（KF 数学在 EkfMeasUpdate）
%
% 观测模型 h(x) = C_nb(q) * m_NED（与 insMagnetometer 的 z = C_nb*m_NED + MagBias
% 同构），C_nb = C_bn' 为 NED->body 旋转，量纲 uT。15 态 EKF 无磁场/磁偏置状态，
% 参考场 m_NED 取固定参数（SE_EKF_MAG_NED）；硬/软磁畸变与场强比例失配不可估。
%
% 误差状态 delta_theta 取 NED 系（左乘流形 retraction：q_c = dq ⊗ q_p，
% 在 EkfMeasUpdate 内统一实现）：
%   C_nb_true = C_nb_est * (I - skew(delta_theta))
%   h_true = C_nb_true * m_NED 约等于 h_est + C_nb_est * skew(m_NED) * delta_theta
%   => H(1:3, 7:9) = C_nb * skew_sym(m_NED)
%
% 野值门限 gate2：马氏距离平方 d2 = innov' S^-1 innov；3D 门限取 χ²(3,0.999)≈16。
% 调用方负责在 FuseMask(3) 为假时保持 x/P（本函数不判断）。
% 状态布局 16 维：[p(3); v(3); q(4); ba(3); bg(3)]，四元数 [w;x;y;z]。
%#codegen
    C_bn = quat2rotm(x_p(7:10));
    C_nb = C_bn.';                       % NED -> body
    h = C_nb * magNed;                   % 预测体轴磁场 [uT]

    H = zeros(3, 15, 'like', x_p);
    H(1:3, 7:9) = C_nb * skew_sym(magNed);
    R = rMag * eye(3, 'like', rMag);

    [x_c, P_c] = EkfMeasUpdate(x_p, P_p, magBody, h, H, R, gate2);
end
