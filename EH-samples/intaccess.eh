#!/usr/bin/ehi
# Illustrates access to integers and strings using the arrow operator
echo 'Int access:'
$ bar = 1
for (count $bar) count i
	echo (@string $i) + ': ' + @string ($bar -> $i)
endfor
echo 'Int modification:'
for (count $bar) count i
	$ bar -> $i = 1
endfor
echo $bar
echo 'String access:'
$ foo = 'test'
for (count $foo) count i
	echo (@string $i) + ': ' + @string ($foo -> $i)
endfor
echo 'String modification:'
for (count $foo) count i
	$ foo -> $i = 117
endfor
echo $foo
