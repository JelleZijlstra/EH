#!/usr/bin/ehi

include '../lib/library.eh'

private it = Iterable.chain('foo', 'bar', 'baz')

for i in it
	echo i
end

echo(it.reverse())
