# List class
enum List
	Nil, Cons(head, tail)

	scope.inherit Iterable

	const empty = () => Nil

	const operator-> = n => match this
		case Cons(@hd, @tl); match n
			case 0; hd
			case 1; tl
			case _; throw(ArgumentError("Argument must be 0 or 1", "List.operator->", n))
		end
	end

	const isEmpty = () => this == Nil

	const isSingleton = () => match this
		case (Nil | Cons(_, Nil)); true
		case _; false
	end

	const map = f => match this
		case Nil; Nil
		case Cons(@hd, @tl); Cons(f hd, tl.map f)
	end

	public reduce = b, f => match this
		case Nil; b
		case Cons(@hd, @tl); f(hd, tl.reduce(b, f))
	end

	public zip = f, rhs => match this, rhs
		case Nil, Nil; Nil
		case (_, Nil) | (Nil, _); throw(ArgumentError.new("Lists are not of equal length", "List.zip", (this, rhs)))
		case Cons(@lhd, @ltl), Cons(@rhd, @rtl); Cons(f(lhd, rhd), ltl.zip(f, rtl))
	end

	const length = () => this.reduce(0, (k, rest => rest + 1))

	const toString() = if this == Cons
		"Cons(head, tail)"
	else
		this.reduce("[]", (k, rest => (k.toString() + "::" + rest)))
	end

	const filter = f => this.reduce(Nil, (elt, accum => match (f elt)
		case true; Cons(elt, accum)
		case false; accum
	end))

	const public reverse_append = accum => match this
		case Nil; accum
		case Cons(@hd, @tl); tl.reverse_append(Cons(hd, accum))
	end

	const reverse = () => this.reverse_append Nil

	# Merge sort implementation
	const public split = l, r => match this
		case Nil; l, r
		case Cons(@hd, @tl); tl.split(r, Cons(hd, l))
	end

	const public merge = r => match this, r
		case Nil, Nil; Nil
		case (@l, Nil) | (Nil, @l); l
		case Cons(@lhd, @ltl), Cons(@rhd, @rtl);
			if lhd < rhd
				Cons(lhd, ltl.merge r)
			else
				Cons(rhd, this.merge rtl)
			end
	end

	const sort = () => match this
		case Nil; Nil
		case Cons(_, Nil); this
		case Cons(@hd, @tl)
			private const l, r = this.split(Nil, Nil)
			l.sort().merge(r.sort())
	end

	const rev_append rhs = match this
		case Nil; rhs
		case Cons(@hd, Nil); Cons(hd, rhs)
		case Cons(@hd, @tl); tl.rev_append(Cons(hd, rhs))
	end

	const concat rhs = this.reverse().rev_append rhs

	const append rhs = this.concat(Cons(rhs, Nil))

	# Count list members for which f elt returns true
	const count = f => this.reduce(Nil, (base, val => match (f val)
		case true; base + 1
		case false; base
	end))

	const join = glue => this.reduce(null, (val, base => match base
		case null; val.toString()
		case _; val.toString() + glue + base
	end))

	class Iterator
		private l

		public initialize = l => (this.l = l)

		public hasNext = () => this.l != Nil

		public next() = if this.l == Nil
			throw(EmptyIterator())
		else
			out, this.l = this.l
			out
		end
	end
	const Iterator = Iterator

	const getIterator() = this.Iterator.new this

	const add v = Cons(v, this)

	const static makeRandom n = if n == 0; Nil; else Random.rand()::(makeRandom (n - 1)); end

	const isSorted() = match this
		case Nil; true
		case Cons(_, Nil); true
		case Cons(@hd1, Cons(@hd2, @tl)); hd1 < hd2 and Cons(hd2, tl).isSorted()
	end
end

# Constify it
const List = List

const Nil = List.Nil
const Cons = List.Cons
Object##operator:: rhs = Cons(this, rhs)

# Conversion methods
Iterable##toList() = this.reduce(Nil, Cons)

Range.empty() = Nil
