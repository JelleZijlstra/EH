#!/usr/bin/ehi

class A
	public aMethod = () => echo "Method from A"
end

class B
	this.inherit A

	private privateMember

	public initialize = func: x
		this.privateMember = x
		echo "Initializing B"
	end

	public bMethod = func:
		echo 'Method from B'
		this.privateMember
	end
end

# instantiate a B; will call B.initialize
b = B.new 3 # prints "Initializing B"

# access B method and print result
echo(b.bMethod()) # prints "Method from B", then "3" (value of b.privateMember)

# access inherited method
b.aMethod() # prints "Method from A"
