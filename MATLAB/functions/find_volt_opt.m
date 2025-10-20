function [OF] = find_volt_opt(var, phi, param)
mode           = param.mode;
group          = param.group;

N_unit          = length(var)/2;            
ue_tmp          = var(1:N_unit);
um_tmp          = var(N_unit+1:N_unit*2);

ue             = repelem(ue_tmp, group);
um             = repelem(um_tmp, group);

[C, err] = volt2coef(ue, um, param.S11, param.S21, param.num_UE, param.num_UM, param.boards, mode);

C = repmat(C, [param.n, 1]);
C = reshape(C, 1,[]);

if err == 1 || any(isnan(C)) 
    OF = 0;
else 
    OF = -abs(sum(C.*exp(1i*phi)));
end