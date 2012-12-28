#!/usr/bin/ehi
include '../lib/library.eh'

assert(true.isA Bool, "is it?")
assert(false.isA Bool, "this one too")
assert(true, "I hope true is true")
assert(!false, "And false isn't")

# @method initialize
assert(Bool.new 1, "should be true")
assert(!(Bool.new 0), "should be false")

# @method toString
assert(true.toString() == "true", "string conversion")
assert(false.toString() == "false", "string conversion")

# @method toBool
assert(true.toBool() == true, "still true")
assert(false.toBool() == false, "still false")

# @method toInt
assert(true.toInt() == 1, "true = 1")
assert(false.toInt() == 0, "false = 0")

# @method operator!
assert((!true) == false, "that's how it works")
assert((!false) == true, "not false is true")
