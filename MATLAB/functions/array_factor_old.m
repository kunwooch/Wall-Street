function [af_gt, af_opt] = array_factor(C_gt1, C_gt2, C_tra, C_ref, param)

% build the array factor, relative feed coefficients  (both ground-truth & huygens)
theta_zero                      = 90; % theta zero direction (90 degree for braodside, 0 degree for endfire.)
af_gt                           = zeros(1,360);    
af_opt                          = zeros(1,360);                             

%% Single beam 
if strcmp(param.mode, 'transmission') || strcmp(param.mode, 'reflection')
    for theta_idx=1:180
        for unit = 0:param.m-1
            af_gt(1,theta_idx)      = af_gt(1,theta_idx) + C_gt1(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
            af_opt(1,theta_idx)     = af_opt(1,theta_idx) + C_tra(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
        end
        af_gt(1,theta_idx)          = abs(af_gt(1,theta_idx));
        af_opt(1,theta_idx)         = abs(af_opt(1,theta_idx));
    end
    for theta_idx=181:360
        for unit = 0:param.m-1 
            af_gt(1,theta_idx)      = af_gt(1,theta_idx) + C_gt2(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
            af_opt(1,theta_idx)     = af_opt(1,theta_idx) + C_ref(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
        end
        af_gt(1,theta_idx)          = abs(af_gt(1,theta_idx));
        af_opt(1,theta_idx)         = abs(af_opt(1,theta_idx));
    end
elseif (strcmp(param.mode, 'bidirection') || strcmp(param.mode, 'transflect')) && ~param.half
    %% Double Beam - Bidirectional
    for theta_idx=1:180
        for unit = 0:param.m-1
            af_gt(1,theta_idx)      = af_gt(1,theta_idx) + C_gt1(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
            af_opt(1,theta_idx)     = af_opt(1,theta_idx) + C_tra(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
        end
        af_gt(1,theta_idx)          = abs(af_gt(1,theta_idx));
        af_opt(1,theta_idx)         = abs(af_opt(1,theta_idx));
    end
    for theta_idx=181:360
        for unit = 0:param.m-1 
            af_gt(1,theta_idx)      = af_gt(1,theta_idx) + C_gt2(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
            af_opt(1,theta_idx)     = af_opt(1,theta_idx) + C_ref(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
        end
        af_gt(1,theta_idx)          = abs(af_gt(1,theta_idx));
        af_opt(1,theta_idx)         = abs(af_opt(1,theta_idx));
    end
elseif (strcmp(param.mode, 'bidirection') || strcmp(param.mode, 'transflect')) && param.half
    %% Double Beam - Bidirectional
    for theta_idx=1:180
        for unit = 0:param.m-1
            if unit < param.m/2
                af_gt(1,theta_idx)      = af_gt(1,theta_idx) + C_gt1(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
            end
            af_opt(1,theta_idx)     = af_opt(1,theta_idx) + C_tra(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
        end
        af_gt(1,theta_idx)          = abs(af_gt(1,theta_idx));
        af_opt(1,theta_idx)         = abs(af_opt(1,theta_idx));
    end
    for theta_idx=181:360
        for unit = 0:param.m-1 
            if unit < param.m/2
                af_gt(1,theta_idx)      = af_gt(1,theta_idx) + C_gt2(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
            end
            af_opt(1,theta_idx)     = af_opt(1,theta_idx) + C_ref(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
        end
        af_gt(1,theta_idx)          = abs(af_gt(1,theta_idx));
        af_opt(1,theta_idx)         = abs(af_opt(1,theta_idx));
    end
elseif strcmp(param.mode, 'combine')
    %% Double Beam - Combine
    for theta_idx=1:180
        for unit = 0:param.m-1
            af_gt(1,theta_idx)      = af_gt(1,theta_idx) + C_gt1(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero)))) ...
                                                         + C_gt2(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
            af_opt(1,theta_idx)     = af_opt(1,theta_idx) + C_tra(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
        end
        af_gt(1,theta_idx)          = abs(af_gt(1,theta_idx));
        af_opt(1,theta_idx)         = abs(af_opt(1,theta_idx));
    end
    for theta_idx=181:360
        for unit = 0:param.m-1 
            af_opt(1,theta_idx)     = af_opt(1,theta_idx) + C_ref(1,unit+1)*exp(1i*unit*param.k*param.side2_e*(cos(deg2rad(theta_idx))-cos(deg2rad(theta_zero))));
        end
        af_gt(1,theta_idx)          = abs(af_gt(1,theta_idx));
        af_opt(1,theta_idx)         = abs(af_opt(1,theta_idx));
    end
end

end