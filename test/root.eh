#!/usr/bin/ehi
# A square root algorithm
func abs: input
	# Without the 0.0, this will instead compare the Float and Integer types. This should really be fixed in the C++ code.
	if input < 0.0
		ret -1 * input
	else
		ret input
	end
end
func sqrt: input
	const epsilon = 0.01
	input = @float input
	out = @float 5
	while (abs (out * out - input)) > epsilon
		out = (out + input/out) / 2.0
	end
	ret out
end

if argc == 2
	num = @int argv->1
else
	num = 27
end
echo sqrt num
