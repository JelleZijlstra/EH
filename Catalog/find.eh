#!/usr/bin/ehi
# Test how many results a given regex generates.
func testregex: regex
	$ rcount = `bfind --title=$regex --return='count' --quiet`
	echo (@string $rcount) + ' results for regex ' + $regex
endfunc
