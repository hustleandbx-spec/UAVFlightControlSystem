function q_new = quat_exp(omega, dt)
%#codegen
% 指数映射: 角速度矢量 → 增量四元数
% omega: 3x1 角速度 (rad/s), dt: 时间步长 (s)
delta = omega * dt;
theta = sqrt(delta(1)^2+delta(2)^2+delta(3)^2);
q_new = zeros(4, 1, 'like', omega);
if theta > 1e-12
    inv_th = 1/theta; axis = delta * inv_th;
    q_new(1) = cos(theta/2);
    q_new(2:4) = axis * sin(theta/2);
else
    q_new(1) = 1;
end
end
