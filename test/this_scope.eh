#!/usr/bin/ehi

# In global context (both are GlobalObject)
echo(this.type())
echo(scope.type())

# in class context (both are A)
class A
	echo(this.type())
	echo(scope.type())
end

# in function context (this is GlobalObject, scope is Function)
f = func:
	echo(this.type())
	echo(scope.type())
end
f()

# and in a method (now this is A)
A.f = f
A.f()
