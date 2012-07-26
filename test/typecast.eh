#!/usr/bin/ehi
# To illustrate typecasting
foo = '3'
bar = @int foo
$echo bar
baz = 3
foobar = @string baz
$echo foobar
more = @string baz
$echo 'If typecasting works, the following line should print 33:'
$echo foobar + more
if foobar == more
	$echo 'foobar and more are the same'
endif
if foobar == baz
	$echo 'That is not true'
endif
if 3 == ('3'.toInt:)
	$echo 'Type juggling works!'
endif
if 3 == '3'
	$echo 'Strict comparison does not work'
else
	$echo 'Strict comparison works too!'
endif
