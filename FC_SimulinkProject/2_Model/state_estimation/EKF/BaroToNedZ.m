function zBaro = BaroToNedZ(pressure, qnh, refAlt, hScale, isaExp)
%BaroToNedZ 静压经 ISA 反算为 NED 下向位置
% 与 INS_EKF.slx 的 BaroToNedZ 同构：BarometerBus 只给静压，高度是估计器侧派生物。
%#codegen

    zBaro = single(refAlt);
    p = double(pressure);
    if p > 1.0
        h = double(hScale) * (1.0 - (p / double(qnh)) ^ double(isaExp));
        zBaro = single(double(refAlt) - h);
    end
end
