#!/usr/bin/ehi
echo include 'included.eh'
echo libraryfunc 42
bar = Foo.new ()
bar.foo ()
# This tries to include a file that doesn't exist
echo include 'nosuchthing'
