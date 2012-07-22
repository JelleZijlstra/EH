#!/usr/bin/ehi
# You can do this, but do you really want to?
$String.operator_times = func: n {
	if n < 1 {
		echo 'Invalid argument'
		ret null
	}
	out = ''
	for n {
		out = out + self
	}
	ret out
}
printvar: "true" * 3
