#!/usr/bin/ehi

class A
	static public a = "value from A"
	public b
end

class B
	this.inherit A

	try
		echo a
	catch if exception.isA NameError
		echo "There is no a"
	end

	# In a previous version of the program, "a" would access different variables in the next two lines
	a = "value from B"
	echo a
	echo(A.a)
end
