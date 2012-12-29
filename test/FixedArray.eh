#!/usr/bin/ehi
include '../lib/library.eh'

private fa = FixedArray.new 5
assert(fa.isA FixedArray)

# @method initialize
assertThrows((() => FixedArray.new -5), ArgumentError, "Must throw an exception on negative size")
private fa2 = FixedArray.new 42
assert(fa2.size() == 42, "size was set to 42")

# @method operator->
assert(fa->0 == null, "Members are initialized to null")
fa->0 = ""
assert(fa->0 == "", "Should be able to set member")
assertThrows((() => fa->5), ArgumentError, "out of range")

# @method operator->=
assert((fa->1 = 3) == 3, "should return rvalue")
assert(fa->1 == 3, "value set correctly")
assertThrows((() => (fa->6 = 3)), ArgumentError, "out of range")

# @method size
assert(fa.size() == 5, "size is still 5")
assert(fa2.size() == 42, "size is still 42")
