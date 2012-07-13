#!/usr/bin/ehi
# To illustrate EH array syntax and the count operator
foo = [ 4, 3, 2 ]
bar = [ 'test', 'foo', 'bar']
echo [1, 2] -> 0
echo $foo -> 0
echo [ 'booh', 2] -> 1
echo $bar -> 1
bar->3 = 'meh'
echo $bar -> 4
echo $bar -> 3
echo $bar -> 2
echo $bar -> 1
echo $bar -> 0
baz = 'muh'
# prints 117 (ASCII value of u)
echo ($baz) -> 1
echo 'muh' -> 1
echo $baz->1
baz = 1
# prints 0
echo $baz -> 0
# prints 1
echo $baz -> 31
echo count $baz
echo count ($bar -> 3)
echo count $bar
empty = []
printvar: $empty
