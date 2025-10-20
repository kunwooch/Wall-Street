function [C, err] = volt2coef(ue, um, S11, S21, num_UE, num_UM, boards, mode)
[x_grid, y_grid]   = meshgrid(num_UE, num_UM);
C = zeros(1, length(boards));
meas_num = unique(boards);

for idx = 1:length(meas_num)
    meas_idx = meas_num(idx);
    % find the group of boards that were measured together for VNA
    board_grp = find(boards==meas_idx);
    [r, t, err] = voltage_sparam_mapping(ue(board_grp), um(board_grp), squeeze(S11(meas_idx,:,:)), squeeze(S21(meas_idx,:,:)), x_grid, y_grid);

%     plot_huygens(squeeze(S11(meas_idx,:,:)), squeeze(S21(meas_idx,:,:)), num_UE, num_UM, 26);

    if strcmp(mode,'transmission')
        C(1,board_grp) = t;
    else
        C(1,board_grp) = r;
    end
end
