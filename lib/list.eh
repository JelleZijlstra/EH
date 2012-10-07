# List class
class List
	private l

	# This is hackish
	const empty = func:
		private old_initialize = List.initialize
		List.initialize = () => (this.l = null)
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
	
	const length = () => this.reduce (0, (k, rest => rest + 1))
	
	const toString = () => this.reduce "[]", (k, rest => (k.toString()) + "::" + rest)

	const filter = f => this.reduce Nil, func: elt, accum
		if (f elt)
			Cons elt, accum
		else
			accum
		end
	end

	const private reverse_append = func: accum
		if this.l == null
			accum
		else
			(this.l->1).reverse_append (Cons this.l->0, accum)
		end
	end

	const reverse = () => this.reverse_append Nil

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

	const append = rhs => ((this.reverse()).rev_append rhs)

	const countPredicate = f => this.reduce Nil, (base, val => given (f val)
		case true; Cons val, base
		case false; base
	end)

	const join = glue => this.reduce null, (val, base => given base
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
	
	const getIterator = () => this.Iterator.new this
	
	const add = v => Cons v, this
end

# Constify it
const List = List

const Nil = List.empty()
const Cons = List.new
Object.operator:: = rhs => List.new this, rhs

# Conversion methods
Iterable.toList = () => this.reduce Nil, (val, out => val::out)

Tuple.toList = func:
	out = Nil
	this.each (func: val
		out = val::out
	end)
	# It gets returned in reversed order...
	out
end
