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
