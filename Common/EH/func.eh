// Show basic functionality of EH functions
func test: a, b
	if $a > 0
		$ a = 3
	else
		$ a = 2
	endif
	$ c = $a + $b
	ret $c
endfunc

echo meh

$ c = 3
if $c = 3
	$ d = test: 1, 2
else
	$ d = test: 3, 4
endif
echo $d

echo $c
call test: $c, $d
echo $c

$ e = "test"
$ f = strlen: $e
echo $e
echo $f
$ g = getinput: 5
echo $g
