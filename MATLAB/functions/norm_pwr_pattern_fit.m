function opt = norm_pwr_pattern_fit(q,angles,normalizedRCS)
F = norm_pwr_pattern(q,angles);
opt = sum(abs(transpose(F)-normalizedRCS));