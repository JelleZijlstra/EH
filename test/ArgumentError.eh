#!/usr/bin/ehi
# ArgumentError testing
include '../lib/library.eh'

private thrower = (_ => "foo"->4)

assertThrows(thrower, ArgumentError, "String.operator-> should throw an ArgumentError")

# @method initialize
try
	thrower()
catch if exception.isA ArgumentError
	assert(exception.message.isA String, "Message must be a string")
	assert(exception.method == 'String.operator->', "Exception was thrown by String.operator->")
	assert(exception.value == 4, "Argument was 4")
end
