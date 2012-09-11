#!/usr/bin/ehi
# Test that a list's l property is indeed private
include '../lib/list.eh'

l = Cons 1, Nil
printvar l
echo l.l
