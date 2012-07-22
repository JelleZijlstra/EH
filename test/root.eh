#!/usr/bin/ehi
# A square root algorithm
func abs: in
	if $in < 0
		ret -$in
	else
		ret $in
	end
end
func sqrt: in
	epsilon = 0.01
	in = @float $in
	out = @float 5
	while (abs: ($out * $out - $in)) > $epsilon
		out = ($out + $in/$out) / 2.0
	end
	ret $out
end

if $argc == 2
	num = @int $argv->1
else
	num = 27
end
echo sqrt: $num
