#!/usr/bin/ehi
# This code created an infinite loop in an earlier version of the parser.
foo := 3
bar := &foo
bar := &foo
echo $foo
