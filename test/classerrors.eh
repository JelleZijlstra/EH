#!/usr/bin/ehi
class Foo
	const toString = func:
		'@Foo'
	end

	try
		for i in 5
			const foo = i
		end
	catch
		echo(exception.toString())
	end
	public mah = 0
	for i in 10
		mah++
	end
	echo mah
end
