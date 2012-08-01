#!/usr/bin/ehi
# A square root algorithm
func abs: in
	# Without the 0.0, this will instead compare the Float and Integer types. This should really be fixed in the C++ code.
	if in < 0.0
		ret -1 * in
	else
		ret in
	end
end
func sqrt: in
	epsilon = 0.01
	in = @float in
	out = @float 5
	while (abs (out * out - in)) > epsilon
		out = (out + in/out) / 2.0
	end
	ret out
end

if argc == 2
	num = @int argv->1
else
	num = 27
end
echo sqrt num
