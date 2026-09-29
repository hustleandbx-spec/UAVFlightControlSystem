function [x_c, P_c] = FuseUpdateGps(x_p, P_p, position_ned, velocity_ned, R_pos, R_vel, gate2)
%FuseUpdateGps  GPS 观测模型层（KF 数学在 EkfMeasUpdate）
%
% 观测 z = [Position_NED; Velocity_NED]，h = [x_p(1:3); x_p(4:6)]，
% H = [I3 0; 0 I3]；R = diag(R_pos*ones(1,3), R_vel*ones(1,3))。
%
% 野值门限 gate2：马氏距离平方；6D 门限取 χ²(6,0.999)≈25。
% 调用方负责在 FuseMask(1) 为假时保持 x/P（本函数不判断）。
%#codegen
    z = single([position_ned; velocity_ned]);
    H = zeros(6, 15, 'like', x_p);
    H(1:3, 1:3) = eye(3, 'like', x_p);
    H(4:6, 4:6) = eye(3, 'like', x_p);
    R = diag([R_pos*ones(1,3,'like',R_pos), R_vel*ones(1,3,'like',R_pos)]);
    h = [x_p(1:3); x_p(4:6)];

    [x_c, P_c] = EkfMeasUpdate(x_p, P_p, z, h, H, R, gate2);
end
