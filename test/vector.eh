#!/usr/bin/ehi
# Inheriting from builtin types doesn't actually work, so we have to cheat
class Vector
	private arr

	# necessary to ensure that the object_data of the resulting object is set; segfaults are likely without this
	public initialize() = (this.arr = [])

	public operator->= (index, value) = do
		if !(index.isA Integer)
			throw "Invalid type for argument 0 to Vector.operator->="
		end
		this.arr->index = value
	end

	public operator-> index = do
		if !(index.isA Integer)
			throw "Invalid type for argument 0 to Vector.operator->"
		end
		this.arr->index
	end

	public length() = this.arr.length()
	public push v = this.arr.push v
end

v = Vector.new ()
printvar v
v.push 42
printvar(v->0)
printvar(v.length ())
