#!/usr/bin/ehi
# Test sorting strings
echo "foo" < "bar"
echo "foo" > "bar"

include '../lib/library.eh'
l = Cons "foo", Cons "bar", Cons "baz", Nil
echo l.sort()
