function [OF] = find_phase(var, phase, num_UE, num_UM, S11, S21, mode)
N_unit          = length(var)/2;            
u_e             = var(1:N_unit);
u_m             = var(N_unit+1:N_unit*2);

[origin_x_grid, origin_y_grid]   = meshgrid(num_UE, num_UM);
[r, t, err]                 = voltage_sparam_mapping(u_e, u_m, S11, S21, origin_x_grid, origin_y_grid);
if strcmp(mode,'transmission')
    C = t;
elseif strcmp(mode,'reflection')
    C = r;
end

if err == 1 || any(isnan(C)) 
    OF = 0;
else 
    % want to minimize
%     OF = -abs(C.*exp(1i*-phase));
    OF = -abs(C)*exp(-abs(phase-angle(C)));
end
