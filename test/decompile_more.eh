#!/usr/bin/ehi
# Try some other features that should be decompilable
include '../lib/library.eh'

echo(Tuple.toString.decompile())

echo(Iterable.nth.decompile())

echo(Array.filter.decompile())

f = () => [1, 2, 3], {foo: 'bar'}
echo(f.decompile())

echo(StringSet.toString.decompile())

echo(Statistics.sd.decompile())

echo(Statistics.median.decompile())

f = func:
	class Foo
		public foo = () => 42
	end
end
echo(f.decompile())

f = func:
	$test --foo=3 --bar -adf -kjkl={foo: 'baz'}
end
echo(f.decompile())

echo(rescue.decompile())

f = func:
	try
		echo 42
	catch if "the world" == "coming to an end"
		echo "Oh no!"
	catch
		echo "Need to put something here"
	finally
		echo "Even this won't happen"
	end
end
echo(f.decompile())
