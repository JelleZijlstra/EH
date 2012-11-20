#!/usr/bin/ehi
# io.eh - IO functionalities

class IO
	const public stdin = File.new "/dev/stdin"
	const public stdout = File.new "/dev/stdout"
	const public stderr = File.new "/dev/stderr"
end
