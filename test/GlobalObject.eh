#!/usr/bin/ehi

include '../lib/library.eh'

# @method toString
assert(this.toString() == "(global execution context)")

# @method printvar
# cannot test with assertions, since printvar prints its output and does not return anything useful
# this is not a complete set of tests for all objects
printvar null
printvar 3
printvar []
printvar {}
printvar(3, foo: 'bar')
printvar class
	public foo = 4
	private bar = 3
	static baz = 5
	const quux = 6
	public static const spam = () => 42
end

# @method include

# if something is wrong in include, this should cause const errors, an infinite loop, or something similarly silly
include '../lib/library.eh'
include 'GlobalObject.eh'

# @method pow
assert(pow(2, 2) == 4, "Integers give an integer result")
assert(pow(0, 0) == 1, "0^0 = 1")
assert(pow(4.0, 0.5) == 2.0, "sqrt(4) == 2")
assert(pow(4, 0.5) == 2.0, "mixed Integer/Float gives Float result")

# @method log
assert(log 1 == 0.0, "log 1 = 0")
private approximately_one = log 2.71828182845904523536
assert(approximately_one > 0.999 && approximately_one < 1.001, "log e = 1")

# @method getinput
# can't really test here

# @method throw
try
	throw()
catch
	assert(exception == (), "null was thrown")
end

# @method echo
echo 3
echo(4::Nil)

# @method put
put 3
put(4::Nil)
put "\n"

# @method workingDir
assert(workingDir().doesMatch "^/.*/EH/test$", "tests are run from the test directory")

# @method shell
assert((shell "pwd").trim() == workingDir(), "I think this should hold")

# @method exit
# how do we test this while also testing handleUncaught?
handleUncaught = func: exception
	echo "Oh no! I could not handle this exception"
	exit 1
end

# @method handleUncaught
throw "exception"
