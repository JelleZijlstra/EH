#!/usr/bin/ehi
# Funny things to do with throw
throw = (func:
	old_throw = throw
	func: exception
		echo 'Oh no, an exception!'
		old_throw exception
	end
end) ()
throw 42
