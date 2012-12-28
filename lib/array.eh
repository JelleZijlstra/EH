# Array functions
Array.map = func: f
	out = []
	for (key, value) in this
		out->key = f value
	end
	out
end

Array.reduce = func: base, f
	out = base
	for value in this
		out = f(value, out)
	end
	out
end

Array.each = func: f
	for (key, value) in this
		f(key, value)
	end
end

Array.filter = func: f
	out = []
	for (key, value) in this
		if f(key, value)
			out->key = value
		end
	end
	out
end

Array.toString = func:
	out = "[ "
	for (key, value) in this
		out = out + key + " => " + value + ", "
	end
	out + "]"
end

Array.append = func: item
	length = this.length()
	this->length = item
	this
end

Array.add = Array.append

Array.empty = () => []

