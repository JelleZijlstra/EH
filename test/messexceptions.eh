#!/usr/bin/ehi
# Trying to break the exception mechanism

# Unfortunately, library classes are constant variables
UnknownCommandError = 42

# That is merely funny
UnknownCommandError.toString = func: -> "I don't care"
$print 3

UnknownCommandError.new = 42
$print 3
