#!/usr/bin/ehi
# User-defined uncaught exception handler
handleUncaught = func: exception
	echo('Uncaught top-level exception: ' + exception)
end
a
