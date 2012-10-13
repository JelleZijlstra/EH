#!/usr/bin/ehi

f = func: args
	private argc = args.length()
	if argc > 4
		echo 'Too many arguments!'
	elsif argc < 4
		echo 'Too few arguments!'
	else
		echo 'Exactly the right number of arguments!'
	end
end

f (1, 2)
f (1, 2, 3, 4, 5)
f (1, 2, 3, 4)
