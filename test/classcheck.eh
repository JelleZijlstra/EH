#!/usr/bin/ehi
include '../lib/tuple.eh'
include '../lib/exception.eh'

# Illustrate the isA method, as well as parameter checking for library functions.
class Foo
	public bar = 42
end
bar = Foo.new ()
# true
echo(bar.isA Foo)
# error
rescue(bar.isA)
# error
rescue(() => echo(pow 3))
# error
rescue(() => echo(pow(3, 2, false)))
# false
class Bar; end
echo(bar.isA Bar)
