function [S11_new, S21_new] = HuygensInterp(S11, S21, num_U_M,num_U_E, new_U)
%% interpolate missing data
[origin_x_grid, origin_y_grid] = meshgrid(num_U_M,num_U_E);
[new_x_grid, new_y_grid] = meshgrid(new_U,new_U);

index=isnan(S11);
% [ii,jj]=find(~isnan(S11))
row_index=any(index,2);
col_index=any(index,1);
NewX=origin_x_grid(~row_index,~col_index);
NewY=origin_y_grid(~row_index,~col_index);
NewZ=S11(~row_index,~col_index);
S11_new=interp2(NewX,NewY,NewZ,new_x_grid,new_y_grid);

index=isnan(S21);
row_index=any(index,2);
col_index=any(index,1);
NewX=origin_x_grid(~row_index,~col_index);
NewY=origin_y_grid(~row_index,~col_index);
NewZ=S21(~row_index,~col_index);
S21_new=interp2(NewX,NewY,NewZ,new_x_grid,new_y_grid);