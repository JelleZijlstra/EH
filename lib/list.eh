# List class
class List
	private l

	# This is hackish
	const empty = func:
		private old_initialize = List.initialize
		List.initialize = func: -> (this.l = null)
		private out = List.new()
		List.initialize = old_initialize
		out
	end
	
	initialize = func: head, tail
		if !(tail.isA List)
			throw ArgumentError.new "List tail must be a List", "List.initialize", tail
		else
			this.l = (head, tail)
		end
	end
	
	const head = func: -> this.l->0
	const tail = func: -> this.l->1

	const operator-> = func: n -> given n
		case 0; this.head()
		case 1; this.tail()
		default; throw ArgumentError.new "Argument must be 0 or 1", "List.operator->", n
	end

	const isEmpty = func: -> (this.l == null)

	const isSingleton = func: -> (this.l != null && (this.l->1).isEmpty())
	
	const map = func: f
		if this.l == null
			Nil
		else
			Cons (f this.l->0), (this.l->1).map f
		end
	end
	
	const reduce = func: base, f
		if this.l == null
			base
		else
			f this.l->0, (this.l->1.reduce base, f)			
		end
	end
	
	const length = func: -> (this.reduce (0, func: k, rest -> rest + 1))
	
	const toString = func: -> (this.reduce "[]", func: k, rest -> (k.toString()) + "::" + rest)

	const filter = func: f -> (this.reduce Nil, func: elt, accum
		if (f elt)
			Cons elt, accum
		else
			accum
		end
	end)

	const private reverse_append = func: accum
		if this.l == null
			accum
		else
			(this.l->1).reverse_append (Cons this.l->0, accum)
		end
	end

	const reverse = func: -> (this.reverse_append Nil)

	# Merge sort implementation
	const private split = func: l, r
		if this.l == null
			l, r
		else
			(this.l->1).split r, (Cons this.l->0, l)
		end
	end

	const private merge = func: r
		if this.l == null
			r
		else
			if (r.isEmpty())
				this
			else
				if this.l->0 < r->0
					Cons this.l->0, (this.l->1).merge r
				else
					Cons r->0, this.merge r->1
				end
			end
		end
	end

	const sort = func:
		if this.l == null
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
		if this.l == null
			ret rhs
		end
		if (this.isSingleton())
			Cons this.l->0, rhs
		else
			(this.l->1).rev_append (Cons this.l->0, rhs)
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
	
	class Iterator
		private l

		public initialize = func: l
			this.l = l
		end
		
		public hasNext = func:
			!(this.l.isEmpty())
		end
		
		public next = func:
			out, this.l = this.l
			out
		end
	end
	private const Iterator = Iterator
	
	const getIterator = func: -> (this.Iterator.new this)
	
	const add = func: v -> (Cons v, this)
end

# Constify it
const List = List

const Nil = List.empty()
const Cons = List.new
Object.operator:: = func: rhs -> (List.new this, rhs)

# Conversion methods
Array.toList = func:
	this.reduce (func: out, val -> val::out), Nil
end

Tuple.toList = func:
	out = Nil
	this.each (func: val
		out = val::out
	end)
	# It gets returned in reversed order...
	out
end
