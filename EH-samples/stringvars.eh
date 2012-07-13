#!/usr/bin/ehi
# Examples of string variables in interpreted EH
foo := 'test'
bar := 'test'
if $foo == $bar
	echo 'It works!'
else
	echo 'This should not be happening'
endif
if $foo >= $bar
	echo 'Wrong!'
else
	echo 'There should have been several error messages on the lines before this one'
endif
echo $foo
echo $bar
baz := $foo . $bar
echo 'This should print "testtest":'
echo $baz
