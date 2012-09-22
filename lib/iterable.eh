#!/usr/bin/ehi
# Illustrate the use of iterators, as well as EH's flexibility

class Iterable
	public each = func: f
		i = this.getIterator()
		while (i.hasNext())
			f i.next()
		end
	end
	
	private reduceHelper = func: it, b, f -> given (it.hasNext())
		case true; f (it.next()), (reduceHelper it, b, f)
		case false; b
	end
	
	public reduce = func: b, f -> (reduceHelper (this.getIterator()), b, f)
	
	public iterableLength = func: -> (this.reduce 0, func: v, b -> b + 1)
	
	private foldLeftHelper = func: it, b, f -> given (it.hasNext())
		case true
			b = f (it.next()), b
			foldLeftHelper it, b, f
		case false
			b
	end
	
	public foldLeft = func: b, f -> (foldLeftHelper (this.getIterator()), b, f)

	public map = func: f
		out = this.empty()
		this.foldLeft out, func: v, collection -> (collection.add (f v))
	end
	
	public filter = func: f
		this.foldLeft (this.empty()), func: v, collection -> given (f v)
			case true; collection.add v
			case false; collection
		end
	end
	
	# sorting implementation
	const private split = func: it, l, r -> given (it.hasNext())
		case true
			val = it.next()
			split it, r, (l.add val)
		case false; l, r
	end

	const private merge = func: l, r
		if !(l.hasNext())
			given (r.hasNext())
				case false; this
				case true; (this.add r.next()).merge l, r
			end
		else
			given (r.hasNext())
				case false; (this.add l.next()).merge l, r
				case true
					const private l_head = l.peek()
					const private r_head = r.peek()
					comparison = l_head <=> r_head
					if comparison < 0
						l.next()
						(this.add l_head).merge l, r
					else
						if comparison == 0
							l.next()
							r.next()
							((this.add l_head).add r_head).merge l, r
						else
							r.next()
							(this.add r_head).merge l, r
						end
					end
			end
		end
	end

	const sort = func: -> given (this.iterableLength())
		case 0; this
		case 1; this
		default
			private l
			private r
			l, r = split (this.getIterator()), (this.empty()), (this.empty())
			l = l.sort()
			r = r.sort()
			(this.empty()).merge (l.getIterator()), (r.getIterator())
	end
end

# Dynamic inheritance in iterable library classes
String.inherit Iterable
Range.inherit Iterable
Hash.inherit Iterable
Array.inherit Iterable
Tuple.inherit Iterable
Integer.inherit Iterable
