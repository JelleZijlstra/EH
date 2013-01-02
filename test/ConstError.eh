#!/usr/bin/ehi

include '../lib/library.eh'

private const constant = 3

assertThrows((() => (global.constant = 4)), ConstError, "attempt to set constant")

assert(constant == 3, "It's still 3")

# @method initialize
try
	constant = 4
	assert(false, "should not get here")
catch if exception.isA ConstError
	assert(exception.object.isA Object, "object should be an object")
	assert(exception.name.isA String, "property name should be a string")
catch
	assert(false, "should not get here")
end
