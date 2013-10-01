# Array functions
Array##map f = do
	out = []
	for value in this
		out.push(f value)
	end
	out
end

Array##reduce(base, f) = do
	out = base
	for value in this
		out = f(value, out)
	end
	out
end

Array##each f = for value in this
	f value
end

Array##filter f = do
	out = []
	for value in this
		if f value
			out.push value
		end
	end
	out
end

Array##toString() = do
	out = "["
	for value in this
		out += "" + value + ", "
	end
	out + "]"
end

Array##append = Array##add = Array##push

Array.empty() = []
