clear;clc;
close all
addpath './functions/'
%% import Huygen's pattern
load("./VNA_WallStreet.mat")

num_boards = 76;
boards = ones(1, num_boards);

plot_huygens(squeeze(S11(1, :, :)), squeeze(S21(1, :, :)), num_UE, num_UM, freq);

%% Define parameters
[param, eval] = define_params(freq);
disp(strcat("Gain limit: ", num2str(param.gain_dB)))

param.half                      = 0;
param.mode                      = 'combine';          % transmission or reflection or bidirection or combine
param.dual                      = 0;                  % transmission or reflection or bidirection or combine
param.num_UE                    = num_UE;
param.num_UM                    = num_UM; 
param.boards                    = boards; 
param.S11                       = S11;
param.S21                       = S21;

filename                        = strcat('./SimResults/lookup_table');
if ~exist(filename, 'dir')
    mkdir(filename)
end

%% Split/Combine
UE                          = 0;    
gNB1                        = [-60:10:60];    
gNB2                        = [-60:10:60];  
alpha1                      = 0.5;  
alpha2                      = 0.5;  

ue_list                     = zeros(length(UE), length(gNB1), length(gNB2), param.m);
um_list                     = zeros(length(UE), length(gNB1), length(gNB2), param.m);

count = 0;
for ue_idx = 1:length(UE)
    tic
    fprintf('----------------------------------------------------------------------------------------------------\n')
    for gNB1_idx = 1:length(gNB1)  
         for gNB2_idx = 1:length(gNB2)
            if abs(gNB1(gNB1_idx) - gNB2(gNB2_idx)) >= 15
                count = count + 1;
                 fprintf("UE Angle: %d, gNB 1 Angle: %d, gNB 2 Angle: %d \n", UE(ue_idx), gNB1(gNB1_idx), gNB2(gNB2_idx))
                 
                 if strcmp(param.mode, 'combine')
                     Tx1 = UE(ue_idx);
                     Tx2 = UE(ue_idx);
                     Rx1 = gNB1(gNB1_idx);
                     Rx2 = gNB2(gNB2_idx);
                 elseif strcmp(param.mode, 'bidirection')
                     Tx1 = UE(ue_idx);
                     Tx2 = gNB1(gNB1_idx)+180;
                     Rx1 = gNB2(gNB2_idx);
                     Rx2 = gNB2(gNB2_idx);
                     % Tx1 = gNB2(gNB2_idx)+180;
                     % Tx2 = gNB1(gNB1_idx)+180;
                     % Rx1 = UE(ue_idx)+180;
                     % Rx2 = gNB2(gNB2_idx);    
                 end
                 %% Multi-beam
                 savestr1                                       = strcat(filename, '/UE_', num2str(UE(ue_idx)), '_gNB1_', num2str(gNB1(gNB1_idx)), '_gNB2_', num2str(gNB2(gNB2_idx)) ,'_Loc.fig');
                 [C_gt1, C_gt2, ue, um, phi_nm1, phi_nm2]       = optimize_mmWall_transflect(Tx1, Tx2, Rx1, Rx2, alpha1, alpha2, param, eval, savestr1);

                [C_tra, C_ref, err] = volt2coef_multibeam(ue, um, S11, S21, num_UE, num_UM, boards);

                C_gt1                                           = repmat(C_gt1, [param.n, 1]);
                C_gt2                                           = repmat(C_gt2, [param.n, 1]);
                C_tra                                           = repmat(C_tra, [param.n, 1]);
                C_ref                                           = repmat(C_ref, [param.n, 1]);

                G_gt                                            = abs(sum(sum( C_gt1.*exp(1i*phi_nm1) + C_gt2.*exp(1i*phi_nm2) )))^2;
                G_opt                                           = abs(sum(sum( C_tra.*exp(1i*phi_nm1) + C_tra.*exp(1i*phi_nm2) )))^2;

                ue_list(ue_idx, gNB1_idx, gNB2_idx, :)          = ue; 
                um_list(ue_idx, gNB1_idx, gNB2_idx, :)          = um;
                fprintf("Perfect gain: %d, mmWall gain (opt): %d\n", pow2db(G_gt)/2, pow2db(G_opt)/2)
                [af_gt, af_opt] = array_factor(C_gt1, C_gt2, C_tra, C_ref, param, 0);

                graph_beamsteering(gNB1(gNB1_idx),gNB2(gNB2_idx), C_tra, C_ref, C_gt1, C_gt2, af_opt, af_gt, param);
            end
         end
    end
    toc
end

if sum(ue_list(:)<0) || sum(um_list(:)<0)
    fprintf("ABORT: NEGATIVE VALUE DETECTED")
else
    save(strcat(filename,'.mat'), 'ue_list', 'um_list', 'UE', 'gNB1', 'gNB2', 'param', 'eval', 'freq', 'boards', 'alpha1', 'alpha2', '-v6')
end
