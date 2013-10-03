#!/usr/bin/ehi

include '../lib/library.eh'

thrower = func:
	it = [].getIterator()
	it.next()
end

assertThrows(thrower, EmptyIterator, "iterator should be depleted")

# @method initialize
try
	thrower()
catch if exception.isA EmptyIterator
	assert(exception.toString() == '<EmptyIterator: Empty iterator>')
end
