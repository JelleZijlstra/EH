#!/usr/bin/ehi

include '../lib/library.eh'

Integer.times = f => for this
	f()
end

private b = 3.times

assert(b.isA Binding, "Variable is saved into a binding")
assert(3.times.isA Binding, "accessing a function through a property automatically creates a binding")
assert(Integer.times.isA Binding, "it is also a binding on the class object")

# @method new
assertThrows((() => Binding.new()), MiscellaneousError, "Cannot create a binding directly")

# @method decompile
assert(b.decompile() == "func: f
	for this
		f ()
	end
end", "decompiling a binding works just like decompiling a function")
assert(3.operator+.decompile() == "(args) => (native code)", "Same goes for native functions")

# @method bindTo
private bound = 3.operator+.bindTo 4
assert(bound 3 == 7, "bound to 4")

# @method toString
assert(b.toString() == "f => (user code)", "works just like Function.toString")
assert(3.operator+.toString() == "(args) => (native code)", "same")

# @method operator()
assert(bound.operator() 7 == 11, "this is 4 inside the binding")
assert(3.operator+ 7 == 10, "and now it is 3")

# @method object
assert(bound.object() == 4, "object is 4")

# @method method
assert(bound.method() == Integer.operator+.method(), "Same method")
