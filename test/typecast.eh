#!/usr/bin/ehi
# To illustrate typecasting
foo = '3'
bar = foo.toInt()
echo bar
baz = 3
foobar = baz.toString()
echo foobar
more = baz.toString()
echo 'If typecasting works, the following line should print 33:'
echo(foobar + more)
if foobar == more
	echo 'foobar and more are the same'
end
if foobar == baz
	echo 'That is not true'
end
if 3 == ('3'.toInt ())
	echo 'Type juggling works!'
end
if 3 == '3'
	echo 'Strict comparison does not work'
else
	echo 'Strict comparison works too!'
end
