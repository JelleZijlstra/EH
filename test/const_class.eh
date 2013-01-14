#!/usr/bin/ehi

# Test that a class name cannot overwrite a const variable
const Foo = 4

try
	class Foo; end
catch if exception.isA ConstError
	echo exception
end

echo Foo
