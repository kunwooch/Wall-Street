function [C, ue, um] = optimize_phase(phase, mode, S11, S21, num_UE, num_UM)
%% find voltages for each phase
max_volt                        = 10;

phase_rad                       = deg2rad(phase);
UE                              = zeros(1,1);
UM                              = zeros(1,1);
vars                            = [UE UM];
vars(:)                         = 0;
lb                              = zeros(1,2);
ub                              = ones(1,2).*max_volt;
nvars                           = length(vars);
options                         = optimoptions('ga','MaxGenerations',15,'PlotFcn',{@gaplotbestf,@gaplotrange});
options.InitialPopulationMatrix = vars;
fun                             = @(x)find_phase(x, phase_rad, num_UE, num_UM, S11, S21, mode);
[volts, fval]                   = ga(fun,nvars,[],[],[],[],lb,ub,[],options);

ue                              = volts(1);
um                              = volts(2);
[origin_x_grid, origin_y_grid]  = meshgrid(num_UE, num_UM);
[r, t, err]                     = voltage_sparam_mapping(ue, um, S11, S21, origin_x_grid, origin_y_grid);
if strcmp(mode,'transmission')
    C = t;
else
    C = r;
end

