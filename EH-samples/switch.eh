#!/usr/bin/ehi
# Illustrate EH switch statements
$ foo = $argc
$ bar = null
switch $foo
	case 1; $ bar = 'I did not get command-line options'; endcase
	case 2; $ bar = 'I got one command-line option'; endcase
endswitch
if !$bar
	$ bar = 'I got some other number of command-line options'
endif
echo $bar
