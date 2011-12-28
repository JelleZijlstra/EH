#!/usr/bin/ehi
# Illustrate associative arrays
$ mah = [ 'test' => 'foo' ]
echo $mah -> 'test'
call printvar: $mah
$ foo = [ 'foo' => 'bar', 'baz' => 'bah' ]
call printvar: $foo
$ mah -> 'test' = 2
echo $mah -> 'test'
