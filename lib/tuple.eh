# Tuple functions
Tuple.toString = func: {
	out = '('
	len = this.length()
	for len count i {
		out = out + this->i
		if i != (len - 1) {
			out = out + ', '
		}
	}
	out + ')'
}

Tuple.each = func: f {
	for (this.length()) count i {
		f this->i
	}
}
