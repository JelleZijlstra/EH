# List class
class List
	initialize = func: head, tail
		if !(tail.isA List)
			throw ArgumentError.new "List tail must be a List", "List.initialize", tail
		else
			(head, tail)
		end
	end
	
	# This is hackish
	empty = func:
		real_initialize = List.initialize
		List.initialize = func: -> null
		out = List.new()
		List.initialize = real_initialize
		out
	end
	
	head = func: -> self->0
	tail = func: -> self->1
	
	map = func: f
		if self == null
			Nil
		else
			Cons (f self->0), (self->1).map f
		end
	end
	
	reduce = func: base, f
		if self == null
			base
		else
			f self->0, (self->1.reduce base, f)			
		end
	end
	
	length = func: -> (reduce (0, func: k, rest -> rest + 1))
	
	toString = func:
		if self == null
			"[]"
		else
			((self->0).toString()) + "::" + ((self->1).toString())
		end
	end
end

const Nil = List.empty()
const Cons = List.new
