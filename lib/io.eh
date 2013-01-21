#!/usr/bin/ehi
# io.eh - IO functionalities

class IO
	private descriptors = {}

	private getter = name => (() => if this.descriptors.has name
		this.descriptors->name
	else
		this.descriptors->name = File.new("/dev/" + name)
		this.descriptors->name
	end)

	const public stdin = getter "stdin"
	const public stdout = getter "stdout"
	const public stderr = getter "stderr"
end
