# Array functions
Array.map = func: f {
	out = []
	for (key, value) in this {
		out->key = f value
	}
	ret out
}

Array.reduce = func: base, f {
	out = base
	for ((), value) in this {
		out = f out, value
	}
	ret out
}

Array.each = func: f {
	for (key, value) in this {
		f key, value
	}
}

Array.filter = func: f {
	out = []
	for (key, value) in this {
		if (f key, value) {
			out->key = value
		}
	}
	ret out
}

Array.toString = func: {
	out = "[ "
	for (key, value) in this {
		out = out + key + " => " + value + ", "
	}
	out + "]"
}

Array.append = func: item {
	length = this.length()
	this->length = item
	this
}

Array.add = Array.append

Array.empty = func: -> []

