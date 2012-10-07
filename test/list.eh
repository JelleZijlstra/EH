#!/usr/bin/ehi

include '../lib/library.eh'

l = Cons 1, (Cons 2, (Cons 3, Nil))
echo l.toString()

echo l.length()

echo (l.map 2.operator*).toString()

echo l.reduce 1, (accum, val => accum * val)

echo (l.reverse()).toString()

l2 = Cons 4, (Cons 2, (Cons 3, (Cons 1, Nil)))

echo (l2.sort()).toString()

echo (l2.append (Cons 1, Nil)).toString()
