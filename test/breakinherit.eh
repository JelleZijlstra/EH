#!/usr/bin/ehi
class A
	try
		this.inherit A
	catch
		echo 'Too bad, that did not work'
	end
	
	class A
		public b
	end

	this.inherit A
	public a = A.new()
end
o = A.new()
printvar o
printvar A
