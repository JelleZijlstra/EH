#!/usr/bin/ehi
# Illustrate associative arrays
mah = [ 'test' => 'foo' ]
echo(mah->'test')
printvar mah
foo = [ 'foo' => 'bar', 'baz' => 'bah' ]
printvar foo
mah->'test' = 2
echo(mah->'test')
