function [OF] = find_volt_opt_multibeam(var, phi1, phi2, alpha1, alpha2, param)
mode           = param.mode;
group          = param.group;

N_unit          = length(var)/2;            
ue_tmp          = var(1:N_unit);
um_tmp          = var(N_unit+1:N_unit*2);

ue             = repelem(ue_tmp, group);
um             = repelem(um_tmp, group);

[C_tra, C_ref, err] = volt2coef_multibeam(ue, um, param.S11, param.S21, param.num_UE, param.num_UM, param.boards);

if param.half
    C_tra = repmat(C_tra(:,1:length(C_tra)/2), [param.n, 1]);
    C_tra = reshape(C_tra, 1,[]);
    C_ref = repmat(C_ref(:,length(C_ref)/2+1:end), [param.n, 1]);
    C_ref = reshape(C_ref, 1,[]);
else
    C_tra = repmat(C_tra, [param.n, 1]);
    C_tra = reshape(C_tra, 1,[]);
    C_ref = repmat(C_ref, [param.n, 1]);
    C_ref = reshape(C_ref, 1,[]);
end

if param.dual
    [C_tra2, C_ref2, err] = volt2coef_multibeam(ue, um, param.S11_2, param.S21_2, param.num_UE, param.num_UM, param.boards);

    C_tra2 = repmat(C_tra2, [param.n, 1]);
    C_tra2 = reshape(C_tra2, 1,[]);
    C_ref2 = repmat(C_ref2, [param.n, 1]);
    C_ref2 = reshape(C_ref2, 1,[]);
end

if err == 1 || any(isnan(C_tra)) || any(isnan(C_ref)) 
    OF = 0;
else 
    if ~param.dual
        if strcmp(mode, 'bidirection') && ~param.half          
            OF = -abs(sum(alpha1*C_tra.*exp(1i*phi1)+alpha2*C_ref.*exp(1i*phi2)));
        elseif strcmp(mode, 'bidirection')  && param.half    
            OF = -abs(sum(C_tra.*exp(1i*phi1)+C_ref.*exp(1i*phi2)));
        elseif strcmp(mode, 'combine') || strcmp(mode, 'split')
            OF = -abs(sum(alpha1*C_tra.*exp(1i*phi1)+alpha2*C_tra.*exp(1i*phi2)));
        end
    else
        if strcmp(mode, 'bidirection') 
            OF = -abs(sum(alpha1*C_tra.*exp(1i*phi1)+alpha2*C_ref2.*exp(1i*phi2)));
        elseif strcmp(mode, 'combine') || strcmp(mode, 'split')
            OF = -abs(sum(alpha1*C_tra.*exp(1i*phi1)+alpha2*C_tra2.*exp(1i*phi2)));
        end
    end
end

end