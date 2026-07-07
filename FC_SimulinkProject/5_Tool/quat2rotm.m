function R = quat2rotm(q)
%#codegen
% 四元数 → 旋转矩阵 C_bn (Body to Navigation)
% 约定: [w; x; y; z] (标量在前, 列向量)
w = q(1); x = q(2); y = q(3); z = q(4);
R = zeros(3, 3, 'like', q);
R(1,1) = 1-2*(y^2+z^2);
R(1,2) = 2*(x*y-w*z);
R(1,3) = 2*(x*z+w*y);
R(2,1) = 2*(x*y+w*z);
R(2,2) = 1-2*(x^2+z^2);
R(2,3) = 2*(y*z-w*x);
R(3,1) = 2*(x*z-w*y);
R(3,2) = 2*(y*z+w*x);
R(3,3) = 1-2*(x^2+y^2);
end
