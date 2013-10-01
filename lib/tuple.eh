# Tuple functions
Tuple##toString() = do
	out = '('
	len = this.length()
	for i in len
		out = out + this->i
		if i != len - 1
			out = out + ', '
		end
	end
	out + ')'
end

Tuple##each f = do
	for i in this.length()
		f(this->i)
	end
end

Tuple##sort() = Tuple(this.toList().sort())

Tuple##reverse() = Tuple(this.toList().reverse())

# For use in iterable: map tuple to a mutable array
Tuple.empty() = []
