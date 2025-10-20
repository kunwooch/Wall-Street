function plot_huygens_volts(S11, S21, num_UM, num_UE, ue, um, freq, boards)
 
meas_num = unique(boards);
for idx = 1:length(meas_num)
    meas_idx = meas_num(idx);
    board_grp = find(boards==meas_idx);

    S11_mark = squeeze(S11(meas_idx,:,:));
    S21_mark = squeeze(S21(meas_idx,:,:));

    for point = 1:length(board_grp)
        [ ~, ix1 ] = min( abs(num_UE-ue(board_grp(point)) ) );
        [ ~, ix2 ] = min( abs(num_UM-um(board_grp(point)) ) );
        S11_mark(ix2, ix1) = 0; % (row, col) = (y, x) = (M, E)
        S21_mark(ix2, ix1) = 0; % (row, col) = (y, x) = (M, E)
    end
    plot_huygens(S11_mark, S21_mark, num_UE, num_UM, freq);
end