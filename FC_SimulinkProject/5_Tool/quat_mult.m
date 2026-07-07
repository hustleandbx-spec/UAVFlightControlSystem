function q_out = quat_mult(q1, q2)
%#codegen
% 四元数乘法 q_out = q1 ⊗ q2 (Hamilton 规则)
% 约定: [w; x; y; z] (标量在前, 列向量)
w1 = q1(1); x1 = q1(2); y1 = q1(3); z1 = q1(4);
w2 = q2(1); x2 = q2(2); y2 = q2(3); z2 = q2(4);
q_out = zeros(4, 1, 'like', q1);
q_out(1) = w1*w2 - x1*x2 - y1*y2 - z1*z2;
q_out(2) = w1*x2 + x1*w2 + y1*z2 - z1*y2;
q_out(3) = w1*y2 - x1*z2 + y1*w2 + z1*x2;
q_out(4) = w1*z2 + x1*y2 - y1*x2 + z1*w2;
end
