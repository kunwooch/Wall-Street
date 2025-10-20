function [OF] = find_volt_opt_precompute(var, C_options, phi, boards)
N_unit          = length(var);            
C = zeros(1,N_unit);
for i = 1:N_unit
  C(1,i) =   C_options(boards(i), uint8(var(i)));
end

C = repmat(C, [double(length(phi)/N_unit), 1]);
C = reshape(C, 1,[]);

OF = -abs(sum(C.*exp(1i*phi)));
end


