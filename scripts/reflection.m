function v = reflection( u, n )     % Reflection of u on hyperplane n.
%
% u can be a matrix. u and v must have the same number of rows.

v = u - 2 * n * (n'*u) / (n'*n);
return