#!/usr/bin/ehi
# Illustrates EH references
$ foo = 3
$ bar = &foo
call printvar: $bar
echo $foo + $bar
$ foo = $foo + 2
echo $foo
echo $bar
$ bar = $bar + 2
echo $foo
echo $bar
