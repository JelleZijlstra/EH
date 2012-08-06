#!/usr/bin/ehi
include '../lib/tuple.eh'
include '../lib/exception.eh'

# Illustrate the isA method, as well as parameter checking for library functions.
class Foo
	public bar = 42
end
bar = Foo.new ()
# true
echo bar.isA Foo
# error
echo bar.isA ()
# error
rescue func: -> (echo pow 3)
# error
rescue func: -> (echo pow (3, 2, false))
# false
class Bar{}
echo bar.isA Bar
