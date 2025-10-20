function [af_gt, af, C_gt, C, ue, um, phi_nm] = optimize_mmWall_phase(TxAngle, RxAngle, param, eval, c_list, ue_list, um_list, boards)
%% method 2. optimization
vars                            = ones(1,param.m);
lb                              = ones(1,param.m);
ub                              = ones(1,param.m).*length(c_list);
nvars                           = length(vars);
options                         = optimoptions('ga','MaxGenerations',100,'PlotFcn',{@gaplotbestf,@gaplotrange});
options.InitialPopulationMatrix = vars;
phases                          = reshape(phi_nm, 1,[]);
fun                             = @(x)find_DCvolt_pre_OPT(x, c_list, phases, boards);
[param_idx, fval]               = ga(fun,nvars,[],[],[],[],lb,ub,[],options);

C_OPT                           = zeros(1,length(param_idx));
ue_OPT                           = zeros(1,length(param_idx));
um_OPT                           = zeros(1,length(param_idx));
phase_OPT                       = zeros(1,length(param_idx));
for board_idx = 1:length(param_idx)
    C_OPT(1,board_idx)          = c_list(boards(board_idx), uint8(param_idx(board_idx)));
    ue_OPT(1,board_idx)         = ue_list(boards(board_idx), uint8(param_idx(board_idx)));
    um_OPT(1,board_idx)         = um_list(boards(board_idx), uint8(param_idx(board_idx)));
end
C_OPT                           = repmat(C_OPT, [param.n, 1]);

