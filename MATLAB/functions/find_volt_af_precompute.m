function [OF] = find_volt_af_precompute(var, C_options, steer_angle, k, spacing, boards)
N_unit          = length(var);            
C = zeros(1,N_unit);
for i = 1:N_unit
  C(1,i) =   C_options(boards(i),uint8(var(i)));
end
% need to figure out the way to mask first three board groups (doesn't cover 360-phase) 

AF_pred         = 0;                            
theta           = 90 - steer_angle;

% array factor calculation
for n=0:N_unit-1                            % sum of array factors
    AF_pred = AF_pred + C(1, n+1)*exp(-1i*n*k*spacing*(cos(deg2rad(theta))));
end
AF_pred = abs(AF_pred);


OF = -abs(AF_pred);
end