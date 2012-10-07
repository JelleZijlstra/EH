#!/usr/bin/ehi
# Inheriting from inbuilt types
class Vector
	this.inherit Array
	
	public initialize = () => []
	
	private set = this.operator->=
	
	public operator->= = func: index, value
		if !(index.isA Integer)
			throw "Invalid type for argument 0 to Vector.operator->="
		end
		this.set index, value
	end
	
	private get = this.operator->
	
	public operator-> = func: index
		if !(index.isA Integer)
			throw "Invalid type for argument 0 to Vector.operator->"
		end
		this.get index
	end
end

v = Vector.new ()
printvar v
v->0 = 42
printvar v->0
printvar v.length ()
