#!/usr/bin/ehi

foo: 3, bar = [1, 'foo' => 4]
echo 'first try'
printvar foo
echo bar

foo: 3, bar = [1, 'foxo' => 4]
echo 'second try'
printvar foo
echo bar
