# Tuple functions
Tuple.toString = func:
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

Tuple.each = func: f
	for i in this.length()
		f(this->i)
	end
end

# For use in iterable: map tuple to a mutable array
Tuple.empty = () => []
