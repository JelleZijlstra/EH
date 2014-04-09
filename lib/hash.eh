#!/usr/bin/ehi
# Assumes iterable.eh is also included

Hash##toString() = do
	private it = this.getIterator()
	private out = '{'
	while it.hasNext()
		private item = it.next()
		out += item->0 + ': ' + item->1
		if it.hasNext()
			out += ', '
		end
	end
	out + '}'
end

Hash.empty() = {}

Hash##add pair = do
	this.operator->= pair
	this
end
