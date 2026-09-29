function delta = DeltaFuse(x_b, x_p)
%DeltaFuse 融合链本拍总修正量（观测信息，与时刻无关）
%
% delta = (x_g − x_p) + (x_b − x_g) 的等价形式，供输出预测器同步：
%   - 姿态：四元数相对旋转 q_b ⊗ conj(q_p)（NED 系；四元数差必须非线性，
%     线性减法对四元数无意义）；
%   - 位置/速度/偏置：线性差。
% 无融合拍时 x_b = x_p ⇒ delta 线性部分为 0、姿态为单位四元数，施加无副作用。
%
% 状态布局 16 维：[p(3); v(3); q(4); ba(3); bg(3)]，四元数 [w;x;y;z]。
%#codegen

    delta   = x_b - x_p;
    qb = x_b(7:10);
    qp = x_p(7:10);
    qc = [qp(1); -qp(2); -qp(3); -qp(4)];          % conj(q_p)
    delta(7:10) = quat_normalize(quat_mult(quat_normalize(qb), qc));
end
