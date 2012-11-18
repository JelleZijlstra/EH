#!/usr/bin/ehi
include '../lib/library.eh'

f = a, b => echo(a, b)
f "foo"

f = a, (b, c) => echo(a, b, c)
f(1, (2, 3))
