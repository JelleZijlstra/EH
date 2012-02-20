#!/usr/bin/ehi
func test:
	$ bar = 3
endfunc
echo $argc
call printvar: $argv
call printvar: $argc
call test:
if $argc == 2
	echo 'I got two arguments'
	echo $argv->0
	echo $argv->1
else
	echo 'I did not get two arguments'
	call printvar: @string $argc
	echo 'I got ' . $argc . ' arguments'
	echo $argv->0
endif
