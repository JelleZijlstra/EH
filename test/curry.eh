#!/usr/bin/ehi
# Currying is possible, though not as elegantly as in OCaml
Array##reduce f = do
	arr = this
	start => do
		out = start
		for val in arr
			out = f out val
			echo out
		end
		out
	end
end
printvar ([1, 2].reduce (v => x => v + 1) 0)
