#!/usr/bin/ehi
# Don't use default in match statements; use case _ instead.
# This used to segfault.

match 3
	case 4; echo "it's 4"
	default; 3
end
