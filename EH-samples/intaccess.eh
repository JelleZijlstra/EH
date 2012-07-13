#!/usr/bin/ehi
# Illustrates access to integers and strings using the arrow operator
echo 'Int access:'
bar = 1
for (count $bar) count i
	echo $i . ': ' . $bar->$i
end
echo 'Int modification:'
for (count $bar) count i
	bar->$i = 1
end
echo $bar
echo 'String access:'
foo = 'test'
for (count $foo) count i
	echo $i . ': ' . $foo->$i
end
echo 'String modification:'
for (count $foo) count i
	foo->$i = 117
end
echo $foo
