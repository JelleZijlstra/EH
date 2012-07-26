#!/usr/bin/ehi
# Exploring oddities of EH syntax
foo = 3
class Foo
	public foo:
		$echo 'baz'
	end
end
$echo 'foo'
$echo 'foo' + 'bar'
$echo foo
foo = Foo.new:
baz = foo.foo
baz:
foo.foo:
muh = 'foo'
foo.foo:
baz:
