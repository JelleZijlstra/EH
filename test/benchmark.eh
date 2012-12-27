#!/usr/bin/ehi

include 'mul2.eh'

const private ITERATIONS = 10000

try
	const private index = argv->1.toInt()
	const private f = funcs->index
catch
	echo('Usage: ' + argv->0 + ' [0-' + funcs.length() + ']')
	$quit
end

for ITERATIONS
	f(5)
end
