#!/usr/bin/ehi
# Test how many results a given regex generates.
func testregex: regex
	bfind --title=$regex --return='count' --printresult=0 --print=0 }$ count
	echo @string $count + ' results for regex ' + $regex
endfunc
