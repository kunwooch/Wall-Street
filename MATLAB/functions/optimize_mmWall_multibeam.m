function [C1_gt, C2_gt, ue, um, phi_nm1, phi_nm2] = optimize_mmWall_multibeam(Tx_angle, Rx_angle1, Rx_angle2, alpha1, alpha2, param, eval, method)
max_volt                        = max(param.num_UE);

%% Calculate the coordinate of Tx, Rx, and mmWall
% y-coordinate (e.g. height) remains the same
tx_phi                          = 0;
tx_theta_rad                   	= deg2rad(Tx_angle);
rx_phi1                         = 0;
rx_theta1_rad                   = deg2rad(Rx_angle1);
rx_phi2                         = 0;
rx_theta2_rad                   = deg2rad(Rx_angle2);
theta_zero                      = 90; % theta zero direction (90 degree for braodside, 0 degree for endfire.)

Tx_loc                          = [0, -eval.d1 * sin(tx_theta_rad), -eval.d1 *  cos(tx_theta_rad)];
if strcmp(param.mode,'transmission')
    Rx_loc1                     = [0, eval.d2 *  sin(rx_theta1_rad), eval.d2 *  cos(rx_theta1_rad)];
    Rx_loc2                     = [0, eval.d2 *  sin(rx_theta2_rad), eval.d2 *  cos(rx_theta2_rad)];
elseif strcmp(param.mode,'bidirection')
    Rx_loc1                     = [0, eval.d2 *  sin(rx_theta1_rad), eval.d2 *  cos(rx_theta1_rad)]; % transmission
    Rx_loc2                     = [0, eval.d2 *  sin(rx_theta2_rad), -eval.d2 *  cos(rx_theta2_rad)]; % reflection
else
    Rx_loc1                     = [0, eval.d2 *  sin(rx_theta1_rad), -eval.d2 *  cos(rx_theta1_rad)];
    Rx_loc2                     = [0, eval.d2 *  sin(rx_theta2_rad), -eval.d2 *  cos(rx_theta2_rad)];
end

figure;
for idx = 1:length(param.boards)
    if param.boards(idx) <= 3
        clr = 'r';  % works well for both ref. and tra.
    else
        clr = 'b';  % works for ref.
    end
    scatter3(eval.x_nm(:,idx)*param.side1_e, eval.y_nm(:,idx)*param.side2_e, zeros(param.n,1), 20, clr); hold on
end
scatter3(Rx_loc1(1,1),Rx_loc1(1,2),Rx_loc1(1,3),40,'r'); hold on;
scatter3(Rx_loc2(1,1),Rx_loc2(1,2),Rx_loc2(1,3),40,'b'); hold on;
scatter3(Tx_loc(1,1),Tx_loc(1,2),Tx_loc(1,3),100,'black')
xlim([-eval.d1*1.2,eval.d1*1.2])
ylim([-eval.d2*1.2,eval.d2*1.2])
xlabel('number cells per board')
ylabel('number of boards')

%% Calculate the required phase shifts to steer the beam from Tx to Rx
tx_d_nm                         = sqrt(((eval.x_coordinate_nm-Tx_loc(1)).^2)+((eval.y_coordinate_nm-Tx_loc(2)).^2)+((eval.z_coordinate_nm-Tx_loc(3)).^2));
rx1_d_nm                        = sqrt(((Rx_loc1(1)-eval.x_coordinate_nm).^2)+((Rx_loc1(2)-eval.y_coordinate_nm).^2)+((Rx_loc1(3)-eval.z_coordinate_nm).^2));
rx2_d_nm                        = sqrt(((Rx_loc2(1)-eval.x_coordinate_nm).^2)+((Rx_loc2(2)-eval.y_coordinate_nm).^2)+((Rx_loc2(3)-eval.z_coordinate_nm).^2));

phi_nm1                          = (2*pi*(tx_d_nm+rx1_d_nm)/param.lambda);
phi_nm2                          = (2*pi*(tx_d_nm+rx2_d_nm)/param.lambda2);
phi_nm                           = angle(alpha1*exp(1i*phi_nm1) + alpha2*exp(1i*phi_nm2));
% phi_nm                           = angle(exp(1i*phi_nm1) + exp(1i*phi_nm2));

%% method 1. find the array factor for corresponding angle
phase_shift_tx1                  = -param.k*param.side2_e*sin(tx_theta_rad)*cos(deg2rad(tx_phi));
phase_shift_tx2                  = -param.k2*param.side2_e*sin(tx_theta_rad)*cos(deg2rad(tx_phi));
phase_shift1                    = param.k*param.side2_e*sin(rx_theta1_rad)*cos(deg2rad(rx_phi1));
phase_shift2                    = param.k2*param.side2_e*sin(rx_theta2_rad)*cos(deg2rad(rx_phi2));
phase_shift_sum1                = phase_shift_tx1 + phase_shift1;
phase_shift_sum2                = phase_shift_tx2 + phase_shift2;
% phase_shift                     = angle((1/2)*exp(1i*phase_shift_sum1) + (1/2)*exp(1i*phase_shift_sum2));
phase_gt1                       = unwrap(phase_shift_sum1*(0:param.m-1));
phase_gt2                       = unwrap(phase_shift_sum2*(0:param.m-1));

C1_gt                            = alpha1*exp(1i*phase_gt1);
C1_gt                            = repmat(C1_gt, [param.n, 1]);
C2_gt                            = alpha2*exp(1i*phase_gt2);
C2_gt                            = repmat(C2_gt, [param.n, 1]);

UE                              = zeros(1,param.m/param.group);
UM                              = zeros(1,param.m/param.group);
vars                            = [UE UM];
vars(:)                         = max_volt;
lb                              = zeros(1,param.m*2/param.group);
ub                              = ones(1,param.m*2/param.group).*max_volt;
nvars                           = length(vars);
options                         = optimoptions('ga','MaxGenerations',100,'PlotFcn',{@gaplotbestf,@gaplotrange});
options.InitialPopulationMatrix = vars;
if strcmp(method, "af")
    %% method 1. find the array factor for corresponding angle
    fun                         = @(x)find_volt_af_multibeam(x, Rx_angle1-Tx_angle, Rx_angle2-Tx_angle, alpha1, alpha2, param);
else
    %% method 2. find the Cnm for each meta-atom using optimization
    % find -phi_nm in huygen's pattern
    % phases                      = reshape(phi_nm, 1,[]);
    phases1                      = reshape(phi_nm1, 1,[]);
    phases2                      = reshape(phi_nm2, 1,[]);
    fun                         = @(x)find_volt_opt_multibeam(x, phases1, phases2, alpha1, alpha2, param);
end

[volts, fval]                   = ga(fun,nvars,[],[],[],[],lb,ub,[],options);
N_unit                          = length(volts)/2; 
ue_tmp                          = volts(1:N_unit);
um_tmp                          = volts(N_unit+1:N_unit*2);
ue                              = repelem(ue_tmp, param.group);
um                              = repelem(um_tmp, param.group);


