function [x_c, P_c] = FuseUpdateBaro(x_p, P_p, zBaro, rBaro, gate2)
%FuseUpdateBaro  气压高度观测模型层（KF 数学在 EkfMeasUpdate）
%
% 观测 h(x) = p_D（NED 下向位置，x(3)），H(1,3)=1，R = rBaro。
% 与 INS_EKF.slx 用 correct() 对 Position_D 做标量更新等价；
% 代价同前：气压计常值偏置不可估。
%
% 野值门限 gate2：马氏距离平方；1D 门限取 χ²(1,0.999)≈12。
% 调用方负责在 FuseMask(2) 为假时保持 x/P（本函数不判断）。
%#codegen
    H = zeros(1, 15, 'like', x_p);
    H(1, 3) = 1;
    R = rBaro;
    h = x_p(3);

    [x_c, P_c] = EkfMeasUpdate(x_p, P_p, zBaro, h, H, R, gate2);
end
