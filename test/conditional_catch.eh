#!/usr/bin/ehi
# Conditional catch statements
try
	echo unknown_variable
catch if exception.isA NameError
	echo 'NameError!'
finally
	echo 'Finally!'
end

try
	try
		$hello
	catch if exception.isA NameError
		echo 'inner NameError!'
	catch if exception.isA TypeError
		echo 'inner TypeError!'
	catch if exception.isA UnknownCommandError
		echo 'inner UnknownCommandError'
	catch
		echo 'Some kind of inner error'
	end
	try
		echo abc
	catch if exception == 'not catching it'
		echo 'a not-catching-it exception!'
	end
catch if exception.isA UnknownCommandError
	echo 'UnknownCommandError!'
catch if exception.isA NameError
	echo 'NameError!'
finally
	echo 'Finally done'
end
