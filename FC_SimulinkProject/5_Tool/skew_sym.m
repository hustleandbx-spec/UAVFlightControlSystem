function S = skew_sym(v)
%#codegen
% 3x1 向量的反对称矩阵 (用于叉乘: skew(v)*w = v × w)
S = zeros(3, 3, 'like', v);
S(1,2) = -v(3);
S(1,3) = v(2);
S(2,1) = v(3);
S(2,3) = -v(1);
S(3,1) = -v(2);
S(3,2) = v(1);
end
