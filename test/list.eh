#!/usr/bin/ehi

include '../lib/library.eh'

l = Cons 1, (Cons 2, (Cons 3, Nil))
echo l.toString()

echo l.length()

echo (l.map 2.operator_times).toString()

echo l.reduce 1, func: accum, val -> accum * val
