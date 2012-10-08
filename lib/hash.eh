#!/usr/bin/ehi
# Assumes iterable.eh is also included

Hash.toString = func:
	private it = this.getIterator()
	out = '{'
	while (it.hasNext())
		item = it.next()
		out += item->0 + ': ' + item->1
		if (it.hasNext())
			out += ', '
		end
	end
	out + '}'
end

Hash.empty = () => {}

Hash.add = func: pair
	this.operator->= pair
	this
end
