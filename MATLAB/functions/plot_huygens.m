function plot_huygens(S11, S21, num_UM, num_UE, freqs)

figure;

xtlabel = string(num_UM);
ytlabel = string(num_UE);
xtlabel((mod(num_UM,1)>0.001)) = " ";
ytlabel((mod(num_UE,1)>0.001)) = " ";

colormap hot
subplot(2,2,1);
abs_s11 = abs(S11);
h_refl = heatmap(num_UM,num_UE,abs_s11,'colormap', hot); % heatmap(col==u_e,row==u_m,matrix)
% h_refl = heatmap(num_U_M,num_U_E,abs(S11(num_U_M+1, num_U_E+1)),'colormap', hot); % heatmap(col==u_e,row==u_m,matrix)
h_refl.Title = 'Reflection (|R|)';
h_refl.YLabel = 'U_M (V)';
h_refl.XDisplayLabels = xtlabel;
h_refl.YDisplayLabels = ytlabel;
caxis([0 1])
set(gca,'FontSize',14,'FontName','Times New Roman') % Creates an axes and sets its FontSize to 18
grid off

subplot(2,2,3);
h_refl_ph = heatmap(num_UM,num_UE,rad2deg(angle(S11)),'colormap', hsv);
% h_refl_ph = heatmap(num_U_M,num_U_E,rad2deg(angle(S11(num_U_M+1, num_U_E+1))),'colormap', hsv);
h_refl_ph.Title = 'Reflection Phase (deg)';
h_refl_ph.YLabel = 'U_M (V)';
h_refl_ph.XLabel = 'U_E (V)';
h_refl_ph.XDisplayLabels = xtlabel;
h_refl_ph.YDisplayLabels = ytlabel;
caxis([-180 180])
set(gca,'FontSize',14,'FontName','Times New Roman') % Creates an axes and sets its FontSize to 18
grid off

subplot(2,2,2);
abs_s21 = abs(S21);
h_tran = heatmap(num_UM,num_UE,abs_s21,'colormap', hot);
% h_tran = heatmap(num_U_M,num_U_E,abs(S21(num_U_M+1, num_U_E+1)),'colormap', hot);
h_tran.Title = 'Transmission (|T|)';
% h_tran.YLabel = 'U_M (V)';
% h_tran.XLabel = 'U_E (V)';
h_tran.XDisplayLabels = xtlabel;
h_tran.YDisplayLabels = ytlabel;
caxis([0 1]);
set(gca,'FontSize',14,'FontName','Times New Roman'); % Creates an axes and sets its FontSize to 18
grid off

subplot(2,2,4);
% h_tran_ph = heatmap(num_U_M,num_U_E,rad2deg(wrapToPi(angle(S21)+pi/3)),'colormap', hsv);
% h_tran_ph = heatmap(num_U_M,num_U_E,rad2deg(angle(S21(num_U_M+1, num_U_E+1))),'colormap', hsv);
h_tran_ph = heatmap(num_UM,num_UE,rad2deg(angle(S21)),'colormap', hsv);
h_tran_ph.Title = 'Transmission Phase (deg)';
% h_tran_ph.YLabel = 'U_M (V)';     
h_tran_ph.XLabel = 'U_E (V)';
h_tran_ph.XDisplayLabels = xtlabel;
h_tran_ph.YDisplayLabels = ytlabel;
caxis([-180 180]);
set(gca,'FontSize',14,'FontName','Times New Roman'); % Creates an axes and sets its FontSize to 18
grid off

annotation('textbox', [0 0.9 1 0.1], ...
    'String', num2str(freqs), ...
    'EdgeColor', 'none', ...
    'HorizontalAlignment', 'center');

% figure;
% colormap hot
% surf(num_U_M,num_U_E,abs(S21)); % heatmap(col==u_e,row==u_m,matrix)

% drawnow;
% pause(0.05)

