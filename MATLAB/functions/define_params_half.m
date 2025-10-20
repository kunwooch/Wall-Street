function [param, eval] = define_params_half(freq1, freq2)
param.half          = 1;                % half board reflecting, other half forwarding

param.c             = 299792458;        % m/s
param.f             = freq1 * 10^9;     % 24 GHz
param.lambda        = param.c/param.f;
param.k             = 2*pi/param.lambda;

param.f2            = freq2 * 10^9;     % 24 GHz
param.lambda2       = param.c/param.f2;
param.k2            = 2*pi/param.lambda2;

%% mmWall parameters
param.n             = 28;    % number cells in one rack (row)
param.m             = 76;
param.group         = 2;
param.side1_e       = 0.00366;          % m
param.side2_e       = 0.00260;          % m
param.side1         = param.side1_e * param.n; % 10 cm (row spacing)
param.side2         = param.side2_e * param.m/2; % 20 cm (col spacing)

%% calculate mmWall gain
param.gain_dB         = pow2db((param.side1*param.side2)*4*pi/(param.lambda^2));

param.gain_dB2        = pow2db((param.side1*param.side2)*4*pi/(param.lambda2^2));

%% calculate mmWall element gain
param.gain_e_dB           = pow2db((param.side1_e*param.side2_e)*4*pi/(param.lambda^2));
param.gain_e              = ((param.side1_e*param.side2_e)*4*pi/(param.lambda^2));

param.gain_e_dB2       = pow2db((param.side1_e*param.side2_e)*4*pi/(param.lambda2^2));
param.gain_e2          = ((param.side1_e*param.side2_e)*4*pi/(param.lambda2^2));

%% environment parameters
eval.d1             = 3;                % m
eval.d2             = 3;                % m
eval.d              = eval.d1+eval.d2;  % m

eval.Gt             = 25;               % dB
eval.Gr             = 25;               % dB
eval.noise_floor    = 86;
PLL                 = -10;
VGA                 = 16;
eval.Pt             = PLL + VGA;

%% delay [0,0,0] at the center of mmWall
eval.x_nm1                       = repmat(transpose(-(param.n/2):(param.n/2)-1), [1,param.m]);
eval.y_nm                       = repmat(-floor(param.m/2):floor(param.m/2)-1, [param.n,1]); % controls
eval.x_coordinate_nm            = eval.x_nm1*param.side1_e;
eval.y_coordinate_nm            = eval.y_nm*param.side2_e; 
eval.z_coordinate_nm            = zeros(param.n, param.m);

