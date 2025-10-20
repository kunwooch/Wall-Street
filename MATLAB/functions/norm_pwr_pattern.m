function F = norm_pwr_pattern(q,angle)

F = zeros(length(angle),1);

for angle_idx = 1:length(angle)
    if abs(angle(angle_idx)) >= 0 && angle(angle_idx) < pi/2
        F(angle_idx,1) = cos(angle(angle_idx))^q;
    end
    if abs(angle(angle_idx)) >= pi/2 && angle(angle_idx) <= pi
        F(angle_idx,1) = 0;
    end
end

F = transpose(F);

