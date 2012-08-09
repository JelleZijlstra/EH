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

	const operator_arrow = func: n -> given n
		case 0; head()
		case 1; tail()
		default; throw ArgumentError.new "Argument must be 0 or 1", "List.operator->", n
	end

	const isEmpty = func: -> (self == null)

	const isSingleton = func: -> (self != null && (self->1).isEmpty())
	
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

	const reverse = func: -> (reverse_append Nil)

	# Merge sort implementation
	const private split = func: l, r
		if self == null
			l, r
		else
			(self->1).split r, (Cons self->0, l)
		end
	end

	const private merge = func: r
		if self == null
			r
		else
			if (r.isEmpty())
				this
			else
				if self->0 < r->0
					Cons self->0, (self->1).merge r
				else
					Cons r->0, this.merge r->1
				end
			end
		end
	end

	const sort = func:
		if self == null
			ret Nil
		end
		if (this.isSingleton())
			this
		else
			private const splitList = this.split Nil, Nil
			((splitList->0).sort()).merge ((splitList->1).sort())
		end
	end

	const rev_append = func: rhs
		if self == null
			ret rhs
		end
		if (this.isSingleton())
			Cons self->0, rhs
		else
			(self->1).rev_append (Cons self->0, rhs)
		end
	end

	const append = func: rhs -> ((this.reverse()).rev_append rhs)

	const countPredicate = func: f -> (this.reduce Nil, func: base, val -> given (f val)
		case true; Cons val, base
		case false; base
	end)

	const join = func: glue -> (this.reduce null, func: val, base -> given base
		case null; val.toString()
		default; (val.toString()) + glue + base
	end)
end

# Constify it
const List = List

const Nil = List.empty()
const Cons = List.new

# Conversion methods
Array.toList = func:
	self.reduce (func: out, val -> Cons val, out), Nil
end

Tuple.toList = func:
	out = Nil
	self.each (func: val
		out = Cons val, out
	end)
	# It gets returned in reversed order...
	out
end
