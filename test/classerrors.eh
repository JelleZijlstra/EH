#!/usr/bin/ehi
class Foo {
	const toString = func:
		'@Foo'
	end

	try
		for 5 count i
			const foo = i
		end
	catch
		echo exception.toString()
	end
	public mah = 0
	for 10 count i
		mah++
	end
	echo mah
}
