function f = graph_beamsteering(tx_angle, rx_angle, C_tra, C_ref, C1_gt, C2_gt, AF_opt, AF_gt, param)
teal = [ 0 0.5 0.5];    % maps for unconventional coloring
theta=deg2rad(1:360);

% f = figure('visible','off');
f = figure;

if param.half
    N_unit = param.m;
    % phase comparison
    subplot(2,2,1);
    scatter(1:N_unit/2, wrapTo2Pi(angle(C1_gt(1,:)))); hold on;  % phase_gt
    scatter(N_unit/2+1:N_unit, wrapTo2Pi(angle(C2_gt(1,:)))); hold on;  % phase_gt
    scatter(1:N_unit, wrapTo2Pi(angle(C_tra(1,:)))); hold on;
    scatter(1:N_unit, wrapTo2Pi(angle(C_ref(1,:)))); hold on;
    xlabel('Element #')
    ylabel({'Phase'})
    xlim([1,N_unit]);
    ylim([0,2*pi]);
    title('Element Coefficient Phase')
    set(gca,'YTick',0:pi/2:2*pi) 
    set(gca,'YTickLabel',{'0','pi/2','pi','3*pi/2','2*pi'})
    legend('GTtra', 'GTref', 'Ctra', 'Cref')
    
    % amplitude comparison
    subplot(2,2,2);
    scatter(1:N_unit/2, abs(C1_gt(1,:))); hold on;
    scatter(N_unit/2+1:N_unit, abs(C2_gt(1,:))); hold on;
    scatter(1:N_unit, abs(C_tra(1,:))); hold on;
    scatter(1:N_unit, abs(C_ref(1,:))); hold on;
    xlabel('Element #')
    ylabel('Transmission')
    xlim([1,N_unit]);
    ylim([0,1]);
    title('Element Coefficient Amplitude')
    legend('GTtra', 'GTref', 'Ctra', 'Cref')
    
    [max_val,max_ind] = max(AF_opt(1,1:180));
    delete(findall(gcf,'Tag','stream'));
    annotation('textbox', [0 0.9 1 0.1], ...
    'String', ['Incident Angle: ' num2str(floor(tx_angle)) ' deg, Steering Angle: ' num2str(floor(rx_angle)) ' deg'], ...
    'EdgeColor', 'none', ...
    'HorizontalAlignment', 'center', ...
    'Color',teal, ...
    'Tag','stream')
    
    subplot(2,2,3);
    polarplot(theta,AF_gt(1,:)./(N_unit/2),'LineWidth',2); hold on;
    polarplot(theta,AF_opt(1,:)./N_unit,'LineWidth',2);
    rlim([0 1])
    pax = gca;
    thetaticks(pax,[0 60 120 180 240 300 360])
    thetaticklabels(pax,{'90^o','30^o','-30^o','-90^o','-120^o','120^o'})
    set(gca,'FontSize',12','FontName','Times New Roman') % Creates an axes and sets its FontSize to 18
    legend('GT','OPT')
    title('GT vs OPT (mag)')
    
    
    subplot(2,2,4);
    polarplot(theta,10*log10((abs(AF_gt(1,:))).^2),'LineWidth',2); hold on;
    polarplot(theta,10*log10((abs(AF_opt(1,:))).^2),'LineWidth',2);
    rlim([0 40])
    pax = gca;
    thetaticks(pax,[0 60 120 180 240 300 360])
    thetaticklabels(pax,{'90^o','30^o','-30^o','-90^o','-120^o','120^o'})
    rticks([0 20 40])
    rticklabels({'0dB','20dB','40dB'})
    set(gca,'FontSize',12,'FontName','Times New Roman') % Creates an axes and sets its FontSize to 18
    legend('GT','OPT')
    title('GT vs OPT (dB)')
else
    N_unit = param.m;
    % phase comparison
    subplot(2,2,1);
    scatter(1:N_unit, wrapTo2Pi(angle(C1_gt(1,:)))); hold on;  % phase_gt
    scatter(1:N_unit, wrapTo2Pi(angle(C2_gt(1,:)))); hold on;  % phase_gt
    scatter(1:N_unit, wrapTo2Pi(angle(C_tra(1,:)))); hold on;
    scatter(1:N_unit, wrapTo2Pi(angle(C_ref(1,:)))); hold on;
    xlabel('Element #')
    ylabel({'Phase'})
    xlim([1,N_unit]);
    ylim([0,2*pi]);
    title('Element Coefficient Phase')
    set(gca,'YTick',0:pi/2:2*pi) 
    set(gca,'YTickLabel',{'0','pi/2','pi','3*pi/2','2*pi'})
    legend('GTtra', 'GTref', 'Ctra', 'Cref')
    
    % amplitude comparison
    subplot(2,2,2);
    scatter(1:N_unit, abs(C1_gt(1,:))); hold on;
    scatter(1:N_unit, abs(C2_gt(1,:))); hold on;
    scatter(1:N_unit, abs(C_tra(1,:))); hold on;
    scatter(1:N_unit, abs(C_ref(1,:))); hold on;
    xlabel('Element #')
    ylabel('Transmission')
    xlim([1,N_unit]);
    ylim([0,1]);
    title('Element Coefficient Amplitude')
    legend('GTtra', 'GTref', 'Ctra', 'Cref')
    
    [max_val,max_ind] = max(AF_opt(1,1:180));
    delete(findall(gcf,'Tag','stream'));
    annotation('textbox', [0 0.9 1 0.1], ...
    'String', ['Incident Angle: ' num2str(floor(tx_angle)) ' deg, Steering Angle: ' num2str(floor(rx_angle)) ' deg'], ...
    'EdgeColor', 'none', ...
    'HorizontalAlignment', 'center', ...
    'Color',teal, ...
    'Tag','stream')

    subplot(2,2,3);
    polarplot(theta,AF_gt(1,:)./N_unit,'LineWidth',2); hold on;
    polarplot(theta,AF_opt(1,:)./N_unit,'LineWidth',2);
    rlim([0 1])
    pax = gca;
    thetaticks(pax,[0 60 120 180 240 300 360])
    thetaticklabels(pax,{'90^o','30^o','-30^o','-90^o','-120^o','120^o'})
    set(gca,'FontSize',12','FontName','Times New Roman') % Creates an axes and sets its FontSize to 18
    legend('GT','OPT')
    title('GT vs OPT (mag)')
    
    
    subplot(2,2,4);
    polarplot(theta,10*log10((abs(AF_gt(1,:))).^2),'LineWidth',2); hold on;
    polarplot(theta,10*log10((abs(AF_opt(1,:))).^2),'LineWidth',2);
    rlim([0 40])
    pax = gca;
    thetaticks(pax,[0 60 120 180 240 300 360])
    thetaticklabels(pax,{'90^o','30^o','-30^o','-90^o','-120^o','120^o'})
    rticks([0 20 40])
    rticklabels({'0dB','20dB','40dB'})
    set(gca,'FontSize',12,'FontName','Times New Roman') % Creates an axes and sets its FontSize to 18
    legend('GT','OPT')
    title('GT vs OPT (dB)')
end

% saveas(f, filename);
% fprintf('1 1: %d dB efficiency,  1 2: %d dB efficiency \n',10*log10((abs(C_amp(1)))^2),10*log10((abs(C_amp(2)))^2))