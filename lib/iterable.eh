#!/usr/bin/ehi
# Illustrate the use of iterators, as well as EH's flexibility

class Iterable
	public each f = do
		i = this.getIterator()
		while i.hasNext()
			f(i.next())
		end
	end

	private static reduceHelper = it, b, f => given it.hasNext()
		case true; f(it.next(), reduceHelper(it, b, f))
		case false; b
	end

	public reduce = b, f => reduceHelper(this.getIterator(), b, f)

	public iterableLength = () => this.reduce(0, (v, b => b + 1))

	private static foldLeftHelper = it, base, f => given (it.hasNext())
		case true
			base = f(it.next(), base)
			foldLeftHelper(it, base, f)
		case false
			base
	end

	public foldLeft = base, f => foldLeftHelper(this.getIterator(), base, f)

	public map = f => this.foldLeft(this.type().empty(), (v, collection => collection.add(f v)))

	public filter = f => this.foldLeft(this.type().empty(), (v, collection => given f v
		case true; collection.add v
		case false; collection
	end))

	public reverse() = this.reduce(this.type().empty(), (elt, accum => accum.append elt))

	# default implementation of append just adds
	# types that by default add to the front (i.e., List) should override this method
	public append elt = this.add elt

	# sorting implementation
	const private static split = it, l, r => given it.hasNext()
		case true
			val = it.next()
			split(it, r, l.add val)
		case false; l, r
	end

	const private merge(l, r) = if !(l.hasNext())
		given (r.hasNext())
			case false; this
			case true; this.add(r.next()).merge(l, r)
		end
	else
		given (r.hasNext())
			case false; (this.add(l.next())).merge(l, r)
			case true
				const private l_head = l.peek()
				const private r_head = r.peek()
				const private comparison = l_head <=> r_head
				if comparison < 0
					l.next()
					(this.add l_head).merge(l, r)
				elsif comparison == 0
					l.next()
					r.next()
					((this.add l_head).add r_head).merge(l, r)
				else
					r.next()
					(this.add r_head).merge(l, r)
				end
		end
	end

	const sort() = given (this.iterableLength())
		case 0; this
		case 1; this
		default
			private l, r = split(this.getIterator(), this.type().empty(), this.type().empty())
			l = l.sort()
			r = r.sort()
			this.type().empty().merge(l.getIterator(), r.getIterator())
	end

	const nth n = do
		if n < 0
			throw ArgumentError.new("Argument must be nonnegative", "Iterable.nth", n)
		end
		const private it = this.getIterator()
		private i = 0
		while true
			elt = it.next()
			if i == n
				ret elt
			end
			i++
		end
	end

	const public reduceSingle f = do
		const private it = this.getIterator()
		if !(it.hasNext())
			ret null
		end
		private out = it.next()
		while it.hasNext()
			private val = it.next()
			if f(out, val)
				out = val
			end
		end
		out
	end

	const public max = () => this.reduceSingle (largest, val => largest < val)

	const public min = () => this.reduceSingle (smallest, val => smallest > val)

	const public clone = () => this.map(x => x)

	const public countWithPredicate = f => this.foldLeft(0, (elt, accum => accum + if f elt; 1; else 0; end))

	const public slice(min, max: null) = do
		if min < 0
			private reversed = this.reverse()
			private part = reversed.slice(if max == null; 0; else -1 * max; end, max: -1 * min)
			part.reverse()
		else
			private out = this.type().empty()
			private it = this.getIterator()

			private i = 0
			while i < min
				it.next()
				i++
			end

			if max == null
				while it.hasNext()
					out = out.append(it.next())
				end
			else
				while i < max
					out = out.append(it.next())
					i++
				end
			end
			out
		end
	end

	const private static any_helper f it = if it.hasNext()
		f(it.next()) || any_helper f it
	else
		false
	end

	const public any f = any_helper f (this.getIterator())

	const private static all_helper f it = if it.hasNext()
		f(it.next()) && all_helper f it
	else
		true
	end

	const public all f = all_helper f (this.getIterator())

	const public containsMember elt = this.any (elt.operator==)

	# Returns an iterable that returns the values in each iterator sequentially
	const public static chain = iter => ChainedIterable.new iter
end

class ChainedIterable
	this.inherit Iterable

	private iterables

	public initialize = this.iterables => ()

	public getIterator() = do
		private iter = this.iterables.getIterator()
		if iter.hasNext()
			Iterator.new iter
		else
			DummyIterator.new()
		end
	end

	public static empty() = try
		this.iterables.getIterator().next().type().empty()
	catch
		throw(Exception.new "Cannot instantiate empty object")
	end

	class Iterator
		private outer_iter
		private inner_obj
		private inner_iter

		public initialize iter = do
			this.outer_iter = iter
			this.inner_obj = iter.next()
			this.inner_iter = this.inner_obj.getIterator()
		end

		public hasNext = () => (this.inner_iter.hasNext() || this.outer_iter.hasNext())

		public next() = do
			# if one of them is empty, continue until we find a non-empty one
			while !(this.inner_iter.hasNext())
				this.inner_obj = this.outer_iter.next()
				this.inner_iter = this.inner_obj.getIterator()
			end
			this.inner_iter.next()
		end
	end
end

class DummyIterator
	const public getIterator = () => this

	const public hasNext = () => false

	const public next = () => throw(EmptyIterator.new())

	const public peek = () => throw(EmptyIterator.new())
end

# Dynamic inheritance in iterable library classes
String.inherit Iterable
Range.inherit Iterable
Hash.inherit Iterable
Array.inherit Iterable
Tuple.inherit Iterable
Integer.inherit Iterable
Map.inherit Iterable
