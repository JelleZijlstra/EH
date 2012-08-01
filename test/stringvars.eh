#!/usr/bin/ehi
# Examples of string variables in interpreted EH
foo = 'test'
bar = 'test'
if foo == bar
	echo 'It works!'
else
	echo 'This should not be happening'
endif
# >= never fails; if two operands are of different types, it does stuff that for the user at least is random, but consistent
if foo >= bar
	echo 'This is what is happening at the moment'
else
	echo 'This is reasonable too'
endif
echo foo
echo bar
baz = foo + bar
echo 'This should print "testtest":'
echo baz
