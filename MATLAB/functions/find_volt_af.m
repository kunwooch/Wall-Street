function [OF] = find_volt_af(var, steer_angle, param)
k              = param.k;
spacing        = param.side2_e;
mode           = param.mode;
group          = param.group;

N_unit         = length(var)/2;            
ue_tmp         = var(1:N_unit);
um_tmp         = var(N_unit+1:N_unit*2);

ue             = repelem(ue_tmp, group);
um             = repelem(um_tmp, group);

theta          = 90 - steer_angle;

[C, err] = volt2coef(ue, um, param.S11, param.S21, param.num_UE, param.num_UM, param.boards, mode);

af         = 0;                            
for n=0:N_unit-1 % sum of array factors
    af = af + C(1, n+1)*exp(-1i*n*k*spacing*(cos(deg2rad(theta))));
%     AF_pred = AF_pred + C(1, n+1)*exp(1i*n*k*spacing*(cos(deg2rad(theta))-cos(deg2rad(theta_zero))));
end
af = abs(af);

if err == 1 || any(isnan(C)) 
    OF = 0;
else 
    OF = -abs(af);
end
