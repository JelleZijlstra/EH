#!/usr/bin/ehi
# Illustrate EH switch statements
foo = argc
bar = null
switch foo
	case 1; bar = 'I did not get command-line options'
	case 2; bar = 'I got one command-line option'
endswitch
if bar == null
	bar = 'I got some other number of command-line options'
endif
echo bar
baz = given foo; case 1; 'None'; case 2; 'Got one'; end
echo baz
