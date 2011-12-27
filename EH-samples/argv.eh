#!/usr/bin/ehi
echo $argc
call printvar: $argv
if $argc = 2
	echo 'I got two arguments'
	echo $argv->0
	echo $argv->1
else
	echo 'I did not get two arguments'
	echo $argv->0
endif
