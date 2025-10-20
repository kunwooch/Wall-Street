function [C1_gt, C2_gt, ue, um, phi_nm1, phi_nm2] = optimize_mmWall_combine(Tx_angle1, Tx_angle2, Rx_angle, alpha1, alpha2, param, eval, savestr)
max_volt                        = max(param.num_UE);

%% Calculate the coordinate of Tx, Rx, and mmWall
% y-coordinate (e.g. height) remains the same
tx_phi1                         = 0;
tx_theta1_rad                   = deg2rad(Tx_angle1);
tx_phi2                         = 0;
tx_theta2_rad                   = deg2rad(Tx_angle2);
rx_phi                          = 0;
rx_theta_rad                    = deg2rad(Rx_angle);
theta_zero                      = 90; % theta zero direction (90 degree for braodside, 0 degree for endfire.)

Tx_loc1                         = [0, -eval.d1 *  sin(tx_theta1_rad), -eval.d1 *  cos(tx_theta1_rad)];
Tx_loc2                         = [0, -eval.d1 *  sin(tx_theta2_rad), -eval.d1 *  cos(tx_theta2_rad)];
Rx_loc                          = [0, eval.d2 *  sin(rx_theta_rad), eval.d2 *  cos(rx_theta_rad)];

figure;
clr = ['r', 'g', 'b', 'y', 'c', 'm','k'];
for idx = 1:length(param.boards)
    scatter3(eval.x_nm(:,idx)*param.side1_e, eval.y_nm(:,idx)*param.side2_e, zeros(param.n,1), 20, clr(param.boards(idx))); hold on
end
scatter3(Rx_loc(1,1),Rx_loc(1,2),Rx_loc(1,3),100,'+', 'filled','MarkerFaceColor','g','MarkerEdgeColor','g','MarkerFaceAlpha',1,'MarkerEdgeAlpha',1); hold on;
scatter3(Tx_loc1(1,1),Tx_loc1(1,2),Tx_loc1(1,3),200,'+', 'filled','MarkerFaceColor','r','MarkerEdgeColor','r','MarkerFaceAlpha',1,'MarkerEdgeAlpha',1); hold on;
scatter3(Tx_loc2(1,1),Tx_loc2(1,2),Tx_loc2(1,3),200,'x', 'filled','MarkerFaceColor','b','MarkerEdgeColor','b','MarkerFaceAlpha',1,'MarkerEdgeAlpha',1); hold on;
xlim([-eval.d1*1.2,eval.d1*1.2])
ylim([-eval.d2*1.2,eval.d2*1.2])
xlabel('number cells per board')
ylabel('number of boards')

%% Calculate the required phase shifts to steer the beam from Tx to Rx
tx1_d_nm                        = sqrt(((eval.x_coordinate_nm-Tx_loc1(1)).^2)+((eval.y_coordinate_nm-Tx_loc1(2)).^2)+((eval.z_coordinate_nm-Tx_loc1(3)).^2));
tx2_d_nm                        = sqrt(((eval.x_coordinate_nm-Tx_loc2(1)).^2)+((eval.y_coordinate_nm-Tx_loc2(2)).^2)+((eval.z_coordinate_nm-Tx_loc2(3)).^2));
rx_d_nm                         = sqrt(((Rx_loc(1)-eval.x_coordinate_nm).^2)+((Rx_loc(2)-eval.y_coordinate_nm).^2)+((Rx_loc(3)-eval.z_coordinate_nm).^2));

phi_nm1                         = (2*pi*(tx1_d_nm+rx_d_nm)/param.lambda);
phi_nm2                         = (2*pi*(tx2_d_nm+rx_d_nm)/param.lambda2);

%% method 1. find the array factor for corresponding angle
phase_shift_tx1                 = -param.k * param.side2_e * sin(tx_theta1_rad) * cos(deg2rad(tx_phi1));
phase_shift_tx2                 = -param.k2* param.side2_e * sin(tx_theta2_rad) * cos(deg2rad(tx_phi2));
phase_shift_rx1                 = param.k  * param.side2_e * sin(rx_theta_rad)  * cos(deg2rad(rx_phi));
phase_shift_rx2                 = param.k2 * param.side2_e * sin(rx_theta_rad)  * cos(deg2rad(rx_phi));
phase_shift_sum1                = phase_shift_tx1 + phase_shift_rx1;
phase_shift_sum2                = phase_shift_tx2 + phase_shift_rx2;
% phase_shift                    = angle((1/2)*exp(1i*phase_shift_sum1) + (1/2)*exp(1i*phase_shift_sum2));
phase_gt1                       = unwrap(phase_shift_sum1*(0:param.m-1));
phase_gt2                       = unwrap(phase_shift_sum2*(0:param.m-1));

C1_gt                           = alpha1*exp(1i*phase_gt1);
C1_gt                           = repmat(C1_gt, [param.n, 1]);
C2_gt                           = alpha2*exp(1i*phase_gt2);
C2_gt                           = repmat(C2_gt, [param.n, 1]);

UE                              = zeros(1,param.m/param.group);
UM                              = zeros(1,param.m/param.group);
vars                            = [UE UM];
vars(:)                         = max_volt;
lb                              = zeros(1,param.m*2/param.group);
ub                              = ones(1,param.m*2/param.group).*max_volt;
nvars                           = length(vars);
options                         = optimoptions('ga','MaxGenerations',100,'PlotFcn',{@gaplotbestf,@gaplotrange});
options.InitialPopulationMatrix = vars;


%% method 2. find the Cnm for each meta-atom using optimization
% find -phi_nm in huygen's pattern
phases1                         = reshape(phi_nm1, 1,[]);
phases2                         = reshape(phi_nm2, 1,[]);
fun                             = @(x)find_volt_opt_multibeam(x, phases1, phases2, alpha1, alpha2, param);

[volts, fval]                   = ga(fun,nvars,[],[],[],[],lb,ub,[],options);
N_unit                          = length(volts)/2; 
ue_tmp                          = volts(1:N_unit);
um_tmp                          = volts(N_unit+1:N_unit*2);
ue                              = repelem(ue_tmp, param.group);
um                              = repelem(um_tmp, param.group);

% [C1, C2, err]                   = volt2coef_multibeam(ue, um, S11_1, S21_1, num_UE, num_UM, boards);
% [C1, C2, err]                   = volt2coef_multibeam(ue, um, S11_2, S21_2, num_UE, num_UM, boards);
% C1                              = repmat(C1, [param.n, 1]);
% C2                              = repmat(C2, [param.n, 1]);



