#!/usr/bin/ehi
# Don't use default in match statements; use case _ instead.
# This used to segfault.

match 3
	case 4; echo "it's 4"
	case _; 3
end
