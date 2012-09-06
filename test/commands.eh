#!/usr/bin/ehi
# Do stuff with commands

$echo 42

$put 3
$echo

commands->'say_hello' = func: paras -> (echo 'hello')
$say_hello

commands->'echo' = null
try
	$echo 'hello'
catch
	printvar exception
end

printvar commands.keys()

commands->'give_me_the_answer' = func: paras -> 42

echo ($give_me_the_answer)

$q
