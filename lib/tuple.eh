# Tuple functions
Tuple.toString = func: {
	out = '('
	len = this.length()
	for i in len {
		out = out + this->i
		if i != (len - 1) {
			out = out + ', '
		}
	}
	out + ')'
}

Tuple.each = func: f {
	for i in (this.length()) {
		f this->i
	}
}
