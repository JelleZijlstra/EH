#!/usr/bin/ehi
class Foo
	public static foo() = do
		for i in 3
			private j = i
			echo j
		end
	end
end

Foo.foo()
