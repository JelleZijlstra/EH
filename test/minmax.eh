#!/usr/bin/ehi
include '../lib/library.eh'

str = 'hello'
echo(str.min())
echo(str.max())

l = 1::2::4::3::Nil
echo(l.min())
echo(l.max())

fa = FixedArray.new 2
fa->0 = 3
fa->1 = 4
echo(fa.min())
echo(fa.max())
