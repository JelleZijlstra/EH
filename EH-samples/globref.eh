#!/usr/bin/ehi
# This might or might not work. Since arguments get deleted after a function
# call, this might be problematic.
var := 3
var2 := 3
func foo: n
	global var
	var := &n
	ret $var
end
func bar: n
	global var2
	var2 := &n
end
b := foo: 2
printvar: $b
printvar: $var

bar: 2
printvar: $var2
