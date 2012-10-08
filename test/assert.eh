#!/usr/bin/ehi
include '../lib/assert.eh'

try
	assert false, "This isn't true"
catch if exception.isA AssertionFailure
	echo exception
end

try
	assert 42
catch if exception.isA AssertionFailure
	echo exception
end

assert (this.isA Bool)
