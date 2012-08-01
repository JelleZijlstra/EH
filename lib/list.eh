# List class
class List
	# This is hackish
	const empty = func:
		private old_initialize = List.initialize
		List.initialize = func: -> null
		private out = List.new()
		List.initialize = old_initialize
		out
	end
	
	initialize = func: head, tail
		if !(tail.isA List)
			throw ArgumentError.new "List tail must be a List", "List.initialize", tail
		else
			(head, tail)
		end
	end
	
	const head = func: -> self->0
	const tail = func: -> self->1
	
	const map = func: f
		if self == null
			Nil
		else
			Cons (f self->0), (self->1).map f
		end
	end
	
	const reduce = func: base, f
		if self == null
			base
		else
			f self->0, (self->1.reduce base, f)			
		end
	end
	
	const length = func: -> (reduce (0, func: k, rest -> rest + 1))
	
	const toString = func: -> (reduce "[]", func: k, rest -> (k.toString()) + "::" + rest)

	const filter = func: f -> (reduce Nil, func: elt, accum
		if (f elt)
			Cons elt, accum
		else
			accum
		end
	end)

	const private reverse_append = func: accum
		if self == null
			accum
		else
			(self->1).reverse_append (Cons self->0, accum)
		end
	end

	reverse = func: -> (reverse_append Nil)
end

# Constify it
const List = List

const Nil = List.empty()
const Cons = List.new
