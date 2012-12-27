#!/usr/bin/ehi

# function to work with code that throws an exception
rescue = f => try
	f()
catch
	echo exception
end

class A
	private a = 1
	static b = 2
	const c = 3
	public d = 4
end

private myA = A.new()

# VisibilityError
rescue(() => echo(A.a))

# setting a static variable modifies parent class
myA.b = 5
echo(A.b) # 5

# ConstError
rescue(() => (A.c = 5))

# does not modify parent object
myA.d = 5
echo(A.d) # 4

# NameError
rescue(() => echo(A.e))
