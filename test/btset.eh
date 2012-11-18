#!/usr/bin/ehi
include '../lib/set.eh'

s = BinaryTreeSet.new()
echo s
s.add 3
s.add 4
s.add 2
s.add 2
echo s
echo(s.debug())
