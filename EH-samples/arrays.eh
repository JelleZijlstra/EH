#!/usr/bin/ehi
# To illustrate EH array syntax
$ foo = [ 4, 3, 2 ]
$ bar = [ 'test', 'foo', 'bar']
echo $foo -> 0
echo [ 'booh', 2] -> 1
echo $bar -> 1
$ bar -> 3 = 'meh'
echo $bar -> 4
echo $bar -> 3
echo $bar -> 2
echo $bar -> 1
echo $bar -> 0
$ baz = 'muh'
# prints 117 (ASCII value of u)
echo $baz -> 1
$ baz = 1
# prints 0
echo $baz -> 0
# prints 1
echo $baz -> 31