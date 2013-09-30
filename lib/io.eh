#!/usr/bin/ehi
# io.eh - IO functionalities

class IO
	private static descriptors = {}

	private static getter name () = if this.descriptors.has name
		this.descriptors->name
	else
		this.descriptors->name = File.new("/dev/" + name)
		this.descriptors->name
	end

	const static public stdin = getter "stdin"
	const static public stdout = getter "stdout"
	const static public stderr = getter "stderr"
end
