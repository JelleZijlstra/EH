
Map.toString = func:
	out = "{"
	for key, value in this
		out += (key.toString()) + ": " + value + ", "
	end
	out + "}"
end
