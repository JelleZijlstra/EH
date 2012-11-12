#!/usr/bin/ehi
# Illustrates access to integers and strings using the arrow operator
echo 'Int access:'
bar = 1
for i in (bar.length ()) * 8
	echo (@string i) + ': ' + (bar.getBit i)
end
echo 'Int modification:'
for i in (bar.length ()) * 8
	bar = bar.setBit i, true
end
echo bar
echo 'String access:'
foo = 'test'
for i in (foo.length ())
	echo (@string i) + ': ' + foo->i
end
echo 'String modification:'
for i in foo.length()
	foo->i = 'u'
end
echo foo
