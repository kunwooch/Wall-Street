%% extract normalized element beam pattern
load('mmWallElementBeamPattern.mat')
angle_theta = deg2rad(mmWallbeampattern.Theta);

normalizedRCS = mmWallbeampattern.AbsRCS+abs(max(mmWallbeampattern.AbsRCS));

figure(1);
mag_normalizedRCS = db2mag(normalizedRCS);
angles = mmWallbeampattern.Theta;
subplot(2,1,2)
plot(mmWallbeampattern.Theta,normalizedRCS)
xlim([-360, 360])
ylim([-25, 0])
gain = max(mmWallbeampattern.AbsRCS); % in dB
title('mmWall element pattern (dB)')
hold on;

subplot(2,1,1)
plot(mmWallbeampattern.Theta,db2mag(normalizedRCS))
xlim([-360, 360])
title('mmWall element pattern (mag)')
hold on;

f = @(x)norm_pwr_pattern_fit(x,angle_theta,db2mag(normalizedRCS));
x0 = 1;
options = optimoptions('fminunc','Algorithm','quasi-newton');
[x, fval] = fminunc(f,x0,options);

F = norm_pwr_pattern(x,angle_theta);
angle_deg = rad2deg(angle_theta);

subplot(2,1,1);
plot(angle_deg, F)
xlim([-360, 360])
title('Fitted mmWall element pattern (mag)')

subplot(2,1,2);
beamdB = mag2db(F);
plot(angle_deg, beamdB)
xlim([-360, 360])
ylim([-25, 0])
title('Fitted mmWall element pattern (dB)')

%% norm power pattern
figure(2);
angle = deg2rad(mmWallbeampattern.Theta);
% angle = -2*pi:pi/16:2*pi;

lambda = 0.01249135; % 24 GHz
% x_len = 0.00248; % m
% y_len = 0.00391; % m
x_len = lambda/2;
y_len = lambda/2;
Ge = (4*pi/(lambda^2))*x_len*y_len;
q = (Ge/2)-1;
% q = 3;
F = norm_pwr_pattern(q,angle);

angle_deg = rad2deg(angle);
subplot(2,1,1);
plot(angle_deg, [F])
% plot(angle_deg, [F(1,end:-1:34) F(1,33:end)])
xlim([-360, 360])
title('Conventional antenna element pattern (mag)')
hold on

subplot(2,1,2);
beamdB = mag2db(F);
% beamdB =mag2db([F(1,end:-1:34) F(1,33:end)]);
% beamdB(isinf(beamdB)) = -25;
plot(angle_deg, beamdB)
xlim([-360, 360])
ylim([-25, 0])
title('Conventional antenna element pattern (dB)')
hold on
% save('./../experiments/plot_data/mmWallBeamPattern.mat')