#!/usr/bin/ehi
func test: n
	echo n
	if (n > 10)
		n
	else
		test (test (n + 1))
	end
end
echo(test 0)
