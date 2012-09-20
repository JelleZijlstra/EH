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
	
	public map = func: f
		out = this.empty()
		this.reduce out, func: v, collection -> (collection.add (f v))
	end
	
	public filter = func: f
		this.reduce (this.empty()), func: v, collection -> given (f v)
			case true; collection.add v
			case false; collection
		end
	end
end

# Dynamic inheritance in iterable library classes
String.inherit Iterable
Range.inherit Iterable
Hash.inherit Iterable
Array.inherit Iterable
Tuple.inherit Iterable
Integer.inherit Iterable
