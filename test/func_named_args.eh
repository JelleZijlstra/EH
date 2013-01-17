#!/usr/bin/ehi
# Named arguments
include '../lib/library.eh'

private foo = bar, baz, option: false => if option
	echo(bar, baz)
else
	printvar(bar, baz)
end

foo(3, 4)
foo(3, 4, option: true)
foo(3, 4, option: false)
