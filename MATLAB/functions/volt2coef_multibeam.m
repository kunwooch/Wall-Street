function [C_tra, C_ref, err] = volt2coef_multibeam(ue, um, S11, S21, num_UE, num_UM, boards)
[x_grid, y_grid]   = meshgrid(num_UE, num_UM);
C_tra = zeros(1, length(boards));
C_ref = zeros(1, length(boards));
meas_num = unique(boards);

for idx = 1:length(meas_num)
    meas_idx = meas_num(idx);
    % find the group of boards that were measured together for VNA
    board_grp = find(boards==meas_idx);
    [r, t, err] = voltage_sparam_mapping(ue(board_grp), um(board_grp), squeeze(S11(meas_idx,:,:)), squeeze(S21(meas_idx,:,:)), x_grid, y_grid);
    C_tra(1,board_grp) = t;
    C_ref(1,board_grp) = r;
end
