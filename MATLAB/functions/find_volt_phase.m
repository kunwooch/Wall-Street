function [OF] = find_DCvolt_phase(var, C_options, steer_angle, k, spacing)
N_unit          = length(var);            
C = zeros(1,N_unit);
for i = 1:N_unit
  C(1,i) =   C_options(uint8(var(i)),1);
end

% C = repmat(C, [double(length(phi)/N_unit), 1]);
% C = reshape(C, 1,[]);
AF_pred         = 0;                            
theta           = 90 - steer_angle;

% array factor calculation
for n=0:N_unit-1                            % sum of array factors
    AF_pred = AF_pred + C(1, n+1)*exp(-1i*n*k*spacing*(cos(deg2rad(theta))));
end
AF_pred = abs(AF_pred);


OF = -abs(AF_pred);
% OF = -abs(sum(C.*exp(1i*phi)));
% OF = -sum(abs(C)*exp(-abs(phase-angle(C)));
end