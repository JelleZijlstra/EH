# Array functions
Array.map = func: f
	out = []
	for value in this
		out.push(f value)
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

Array.each f = for value in this
	f value
end

Array.filter = func: f
	out = []
	for value in this
		if f value
			out.push value
		end
	end
	out
end

Array.toString = func:
	out = "["
	for value in this
		out += "" + value + ", "
	end
	out + "]"
end

Array.append = Array.add = Array.push


Array.empty = () => []

