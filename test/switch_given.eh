#!/usr/bin/ehi

private foo = 3

# Simple switch statement
switch foo
	case 3
		echo "foo = 3"
	case 4
		echo "foo = 4"
	default
		echo "foo isn't 3 or 4"
end

# Equivalent given statement
given foo
	case 3
		echo "foo = 3"
	case 4
		echo "foo = 4"
	default
		echo "foo isn't 3 or 4"
end

# However, switch also allows this
switch foo
	case 3
		echo "foo = 3"
		continue
	case 4
		echo "foo = 4"
	default
		echo "foo isn't 4"
end

# It is possible to use functions in the match
given foo
	case (n => n % 2 == 0)
		echo "foo is even"
	case (n => n % 3 == 0)
		echo "foo is divisible by 3"
	default
		echo "foo is not divisible by 2 or 3"
end

# This will not do anything in switch
switch foo
	case 4
		echo "foo is 4"
end

# But this will throw a MiscellaneousError
given foo
	case 4
		echo "foo is 4"
end
