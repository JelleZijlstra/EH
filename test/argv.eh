#!/usr/bin/ehi
func test:
	bar = 3
endfunc
$echo argc
printvar: argv
printvar: argc
test:
if argc == 2
	$echo 'I got two arguments'
	$echo argv->0
	$echo argv->1
else
	$echo 'I did not get two arguments'
	printvar: @string argc
	$echo 'I got ' + argc + ' arguments'
	$echo argv->0
endif
