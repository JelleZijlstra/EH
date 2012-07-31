# Array functions
Array.map = func: f {
	out = []
	for self as key => value {
		out->key = f value
	}
	ret out
}

Array.reduce = func: f, base {
	out = base
	for self as value {
		out = f out, value
	}
	ret out
}

Array.each = func: f {
	for self as key => value {
		f key, value
	}
}

Array.filter = func: f {
	out = []
	for self as key => value {
		if (f key, value) {
			out->key = value
		}
	}
	ret out
}

Array.toString = func: {
	out = "[ "
	for self as key => value {
		out = out + key + " => " + value + ", "
	}
	out + "]"
}
