// Example of an EH input file accepted by the compiler (which does not
// currently accept variables, function calls, or for loops, and even for things
// that it does accept generates spectacularly inefficient assembly code).
// $ a = 3
if 2 = 2
	3 + 3
	echo 2 - 3
endif
if 3 - 3
	echo 2
endif
if 3 > 2
	echo 1
endif
if 2 > 3
	echo 0
endif
if 2 < 3
	echo 1
endif
if 2 > 3
	echo 0
endif
if 2 >= 2
	echo 1
endif
if 2 >= 3
	echo 0
endif
if 2 != 3
	echo 1
endif
if 2 != 2
	echo 0
endif
while 1 != 1
	echo 2
endwhile
while 2 != 2
	echo 1
endwhile
