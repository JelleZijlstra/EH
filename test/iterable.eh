#!/usr/bin/ehi
# Illustrate the use of iterators, as well as EH's flexibility

class Iterable
	public each = func: f
		i = this.getIterator()
		while (i.hasNext())
			f i.next()
		end
	end
end

String.inherit Iterable
"foo".each printvar

Range.inherit Iterable
1..3.each printvar
