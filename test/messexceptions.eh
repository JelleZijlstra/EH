#!/usr/bin/ehi
# Trying to break the exception mechanism

try
	# Unfortunately, library classes are constant variables
	UnknownCommandError = 42
catch
	echo exception
end

try
	# That is merely funny
	UnknownCommandError.toString = () => "I don't care"
	$print 3
catch
	echo exception
end

UnknownCommandError.new = 42
$print 3
