#!/usr/bin/ehi
# Test the unary operators
bar = 0
echo ~bar
foo = 1
echo (-1 * foo)
baz = false
echo !baz
echo !(foo.toBool ())
# Even pathological stuff is allowed by ehi
echo !((~foo * -1).toBool())
