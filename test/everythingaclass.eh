#!/usr/bin/ehi
include '../lib/exception.eh'

const foo = 3
echo foo
# should generate an error
rescue () => (foo = 4)
echo foo
class Bar {
	echo 5
	mah = 42
}
printvar Bar.new()
