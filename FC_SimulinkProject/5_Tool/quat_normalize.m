function q_n = quat_normalize(q)
%#codegen
% 四元数归一化
q_n = zeros(4, 1, 'like', q);
q_n(:) = q / sqrt(q(1)^2+q(2)^2+q(3)^2+q(4)^2);
end
