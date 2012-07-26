#!/usr/bin/ehi
# Test the unary operators
bar = 0
$echo ~bar
foo = 1
$echo (-foo)
baz = false
$echo !baz
$echo !(foo.toBool:)
# Even pathological stuff is allowed by ehi
$echo (!(-~foo.toBool:))
