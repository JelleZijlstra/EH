#!/usr/bin/ehi
include '../lib/exception.eh'

# This is totally crazy, but at least it shouldn't crash ehi.
# (It is significantly less crazy now that we no longer support references.)
foo = 3
bar = 'foo'
rescue func: -> (bar->foo = 1)
printvar bar
printvar foo
rescue func: -> (echo bar->foo)
echo foo->bar
