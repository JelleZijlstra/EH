#!/usr/bin/ehi
func test: n
	echo (foo n)
	printvar (foo n)
	ret (foo n)
end
func foo: n
	echo n
	ret n
end
echo (test 2)
printvar (test 2)
ret 42
