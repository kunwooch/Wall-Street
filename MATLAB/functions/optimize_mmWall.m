function [C_gt, ue, um, phi_nm, phase_shift] = optimize_mmWall(Tx_angle, Rx_angle, param, eval, filename)
max_volt                        = max(param.num_UE);

%% Calculate the coordinate of Tx, Rx, and mmWall
% y-coordinate (e.g. height) remains the same
phi1                            = 0;
theta1_rad                      = deg2rad(Tx_angle);
phi2                            = 0;
theta2_rad                      = deg2rad(Rx_angle);

Tx_loc                          = [0, -eval.d1 *  sin(theta1_rad), -eval.d1 *  cos(theta1_rad)];
if strcmp(param.mode,'transmission')
    Rx_loc                      = [0, eval.d2 *  sin(theta2_rad), eval.d2 *  cos(theta2_rad)];
else
    Rx_loc                      = [0, eval.d2 *  sin(theta2_rad), -eval.d2 *  cos(theta2_rad)];
end

% f = figure('visible','off');
f = figure;
for idx = 1:length(param.boards)
    if param.boards(idx) <= 3
        clr = 'r';  % works well for both ref. and tra.
    else
        clr = 'b';  % works for ref.
    end
    scatter3(eval.x_nm(:,idx)*param.side1_e, eval.y_nm(:,idx)*param.side2_e, zeros(param.n,1), 20, clr); hold on
end
scatter3(Rx_loc(1,1),Rx_loc(1,2),Rx_loc(1,3),40,'r'); hold on;
scatter3(Tx_loc(1,1),Tx_loc(1,2),Tx_loc(1,3),100,'r')
xlim([-eval.d1*1.2,eval.d1*1.2])
ylim([-eval.d2*1.2,eval.d2*1.2])
xlabel('number cells per board')
ylabel('number of boards')
saveas(f, filename);

%% Calculate the required phase shifts to steer the beam from Tx to Rx
d1_nm                           = sqrt(((eval.x_coordinate_nm-Tx_loc(1)).^2)+((eval.y_coordinate_nm-Tx_loc(2)).^2)+((eval.z_coordinate_nm-Tx_loc(3)).^2));
d2_nm                           = sqrt(((Rx_loc(1)-eval.x_coordinate_nm).^2)+((Rx_loc(2)-eval.y_coordinate_nm).^2)+((Rx_loc(3)-eval.z_coordinate_nm).^2));

phi_nm                          = 2*pi*(d1_nm+d2_nm)/param.lambda;

phase_shift1                    = -param.k*param.side2_e*sin(theta1_rad)*cos(deg2rad(phi1));
phase_shift2                    = param.k*param.side2_e*sin(theta2_rad)*cos(deg2rad(phi2));
phase_shift                     = phase_shift1+phase_shift2;
phase_gt                        = unwrap(phase_shift*(0:param.m-1));

C_gt                            = 1*exp(1i*phase_gt);
% C_gt                            = repmat(C_gt, [param.n, 1]);

%% Run optimization
ue                              = zeros(1,param.m/param.group);
um                              = zeros(1,param.m/param.group);
vars                            = [ue um];
vars(:)                         = 0;
lb                              = zeros(1,param.m*2/param.group);
ub                              = ones(1,param.m*2/param.group).*max_volt;
nvars                           = length(vars);
options                         = optimoptions('ga','MaxGenerations',200,'PlotFcn',{@gaplotbestf,@gaplotrange});
options.InitialPopulationMatrix = vars;


%% method 2. find the Cnm for each meta-atom using optimization
% find -phi_nm in huygen's pattern
phases                          = reshape(phi_nm, 1,[]);
fun                             = @(x)find_volt_opt(x, phases, param);

[volts, fval]                   = ga(fun,nvars,[],[],[],[],lb,ub,[],options);   
N_unit                          = length(volts)/2; 
ue_tmp                          = volts(1:N_unit);
um_tmp                          = volts(N_unit+1:N_unit*2);
ue                              = repelem(ue_tmp, param.group);
um                              = repelem(um_tmp, param.group);

% [C1, err]                       = volt2coef(ue, um, S11, S21, num_UE, num_UM, boards, 'transmission');
% [C2, err]                       = volt2coef(ue, um, S11, S21, num_UE, num_UM, boards, 'reflection');
% C1                              = repmat(C1, [param.n, 1]);
% C2                              = repmat(C2, [param.n, 1]);



