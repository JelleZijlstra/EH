#!/usr/bin/ehi
# ArgumentError testing
include '../lib/library.eh'

private thrower = (_ => "foo"->4)

assertThrows(thrower, ArgumentError, "String.operator-> should throw an ArgumentError")

# @method initialize
try
	thrower()
catch if exception.isA ArgumentError
	assert(exception.message.isA String)
	assert(exception.method == 'String.operator->')
	assert(exception.value == 4)
end
