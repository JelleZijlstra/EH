#!/usr/bin/ehi

# Execute a function, wrapped in a try-catch block
private handler = f => try
	f()
catch if exception.isA TypeError
	echo 'TypeError'
catch if exception.isA NameError
	echo 'NameError'
catch
	echo 'Some other kind of error'
finally
	echo 'Finally!'
end

# will throw a NameError
handler(() => echo unknown_variable)

# TypeError
handler(() => 3 << "can't shift by a string")

# MiscellaneousError
handler(() => match 3
	case 4; echo "it's 4"
end)
