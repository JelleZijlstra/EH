#!/usr/bin/ehi
switcher = func: n
	testfunc = func: n
		if !(n.isA Integer)
			echo 'Error: input must be an integer'
			ret false
		end
		if n % 2
			ret false
		else
			ret true
		end
	end
	match n
		case _ when testfunc n
			echo 'This number is even'
		case _
			echo 'This number is odd'
	end
end

switcher 2
switcher 3
