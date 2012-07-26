#!/usr/bin/ehi
# Try to find array access bug
foo = 1
for (foo.length: ()) * 8 count i
	$echo foo.getBit: i
end
$echo foo.getBit: 1
