function [OF] = find_volt_af_multibeam(var, steer_angle1, steer_angle2, alpha, alpha2, S11, S21, boards, param)
k              = param.k;
spacing        = param.side2_e;
mode           = param.mode;
group          = param.group;

N_unit         = length(var)/2;            
ue_tmp         = var(1:N_unit);
um_tmp         = var(N_unit+1:N_unit*2);

ue             = repelem(ue_tmp, group);
um             = repelem(um_tmp, group);

theta1          = 90 - steer_angle1;
theta2          = 90 - steer_angle2;

[C, err] = volt2coef(ue, um, S11, S21, param.num_UE, param.num_UM, boards, mode);
     
af         = 0;                            
for n=0:N_unit-1 % sum of array factors
    af = af + alpha*C(1, n+1)*exp(-1i*n*k*spacing*(cos(deg2rad(theta1)))) + alpha2*C(1, n+1)*exp(-1i*n*k*spacing*(cos(deg2rad(theta2))));
%     AF_pred = AF_pred + C(1, n+1)*exp(1i*n*k*spacing*(cos(deg2rad(theta))-cos(deg2rad(theta_zero))));
end
af = abs(af);

if err == 1 
    OF = 0;
else 
    OF = -abs(af);
end
