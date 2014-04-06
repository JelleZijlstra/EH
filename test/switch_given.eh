#!/usr/bin/ehi

private foo = 3

# Simple switch statement
match foo
	case 3
		echo "foo = 3"
	case 4
		echo "foo = 4"
	case _
		echo "foo isn't 3 or 4"
end

# Equivalent given statement
match foo
	case 3
		echo "foo = 3"
	case 4
		echo "foo = 4"
	case _
		echo "foo isn't 3 or 4"
end

# However, switch also allows this
match foo
	case 3
		echo "foo = 3"
	case 4
		echo "foo = 4"
	case _
		echo "foo isn't 4"
end

# It is possible to use functions in the match
match foo
	case _ when foo % 2 == 0
		echo "foo is even"
	case _ when foo % 3 == 0
		echo "foo is divisible by 3"
	case _
		echo "foo is not divisible by 2 or 3"
end

# This will throw a MiscellaneousError
match foo
	case 4
		echo "foo is 4"
end
