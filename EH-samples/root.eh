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
	set epsilon = 0.01
	set in = @float $in
	set out = @float 5
	while (abs: ($out * $out - $in)) > $epsilon
		set out = ($out + $in/$out) / 2.0
	end
	ret $out
end

if $argc = 2
	set num = @int $argv->1
else
	set num = 27
end
echo sqrt: $num
