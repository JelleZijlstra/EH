# Tuple functions
Tuple.toString = func: {
	out = '('
	len = self.length()
	for len count i {
		out = out + self->i
		if i != (len - 1) {
			out = out + ', '
		}
	}
	out + ')'
}

Tuple.each = func: f {
	for (self.length()) count i {
		f self->i
	}
}
