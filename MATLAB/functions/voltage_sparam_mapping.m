function [r,t,err] = voltage_sparam_mapping(u_e, u_m, S11, S21, x_grid, y_grid)
%% interpolate the S21 from HFSS
S11_amp = abs(S11);
S11_ph = rad2deg(angle(S11));
S21_amp = abs(S21);
S21_ph = rad2deg(angle(S21));
r = 0;
t = 0;
try
    S11_amp_ut = interp2(x_grid,y_grid,S11_amp,u_e, u_m);
    S21_amp_ut = interp2(x_grid,y_grid,S21_amp,u_e, u_m);
    S11_ph_ut  = interp2(x_grid,y_grid,S11_ph,u_e, u_m);
    S21_ph_ut  = interp2(x_grid,y_grid,S21_ph,u_e, u_m);

    nanx = isnan(S11_amp_ut);
    r    = 1:numel(S11_amp_ut);
    S11_amp_ut(nanx) = interp1(r(~nanx), S11_amp_ut(~nanx), r(nanx));

    nanx = isnan(S11_ph_ut);
    r    = 1:numel(S11_ph_ut);
    S11_ph_ut(nanx) = interp1(r(~nanx), S11_ph_ut(~nanx), r(nanx));

    nanx = isnan(S21_amp_ut);
    t    = 1:numel(S21_amp_ut);
    S21_amp_ut(nanx) = interp1(t(~nanx), S21_amp_ut(~nanx), t(nanx));

    nanx = isnan(S21_ph_ut);
    t    = 1:numel(S21_ph_ut);
    S21_ph_ut(nanx) = interp1(t(~nanx), S21_ph_ut(~nanx), t(nanx));

    r = S11_amp_ut.*exp(1j*unwrap(deg2rad(S11_ph_ut)));
    t = S21_amp_ut.*exp(1j*unwrap(deg2rad(S21_ph_ut)));
    
    % temporaily comment these line for size vs gain simulation
%     idx = find(isnan(r(1:10)), 1, 'last' ) + 1;
%     %idx = find(isnan(r(1:5)), 1, 'last' ) + 1;
%     r(isnan(r(1:5))) = r(idx);
%     t(isnan(t(1:5))) = t(idx);
%     idx = find(isnan(r), 1 ) - 1;
%     r(isnan(r)) = r(idx);
%     t(isnan(t)) = t(idx);    
    err = 0;
catch 
   warning('Variable exeeding the constraint');
   err = 1;
end
end