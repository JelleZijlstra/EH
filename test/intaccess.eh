#!/usr/bin/ehi
# Illustrates access to integers and strings using the arrow operator
$echo 'Int access:'
bar = 1
for (bar.length: ()) * 8 count i
	$echo (@string i) + ': ' + (bar.getBit: i)
end
$echo 'Int modification:'
for (bar.length: ()) * 8 count i
	bar = bar.setBit: i, true
end
$echo bar
$echo 'String access:'
foo = 'test'
for (foo.length: ()) count i
	$echo (@string i) + ': ' + foo->i
end
$echo 'String modification:'
for (foo.length: ()) count i
	foo->i = 'u'
end
$echo foo
