# Use EH standard library
include '../lib/library.eh'

# Most verbose form
func mul1: x
	ret 2 * x
end

# This syntax makes it clearer that the code merely sets the variable to hold a function as a value
mul2 = func: x
	ret 2 * x
end

# "ret" is implied and can be omitted
mul3 = func: x
	2 * x
end

# Semicolons can replace all newlines
mul4 = func: x; 2 * x; end

# Short function syntax
mul5 = x => 2 * x

# Even shorter syntax
mul6 x = 2 * x

# Instead of using a function, we can simply use the method directly
mul7 = 2.operator*

# Test that these functions all obey the specification
funcs = (mul1, mul2, mul3, mul4, mul5, mul6, mul7)
tester f = assert(f 3 == 6)
funcs.each tester
