#!/usr/bin/ehi
func libraryfunc: n
	$echo 'This is a library function'
	ret n
end
class Foo
	public foo:
		$echo 'This is a method belonging to a library class'
	end
end
ret 1
