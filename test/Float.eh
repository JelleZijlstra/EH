#!/usr/bin/ehi
# Tests here are likely risky, because they rely on equality of floating-point
# numbers. Perhaps we should add a Float.epsilonEquals method.

include '../lib/library.eh'

private f = 0.0
assert(f.isA Float, "float literal")

# @method initialize
private f2 = Float.new 1
assert(f2 == 1.0, "should have called Integer.toFloat")
private f3 = Float.new "2.0"
assert(f3 == 2.0, "should have called String.toFloat")

# @method operator+
assert(1.0 + 1.0 == 2.0, "1 + 1 = 2")

# @method operator-
assert(1.0 - 1.0 == 0.0, "1 - 1 = 0")

# @method operator*
assert(1.0 * 1.0 == 1.0, "1 * 1 = 1")
assert(2.0 * 2.0 == 4.0, "2 * 2 = 4")

# @method operator/
assertThrows((() => 3.0 / 0.0), MiscellaneousError, "3 / 0 is undefined")
assert(3.0 / 1.0 == 3.0, "3 / 1 = 3")
assert(4.0 / 2.0 == 2.0, "4 / 2 = 2")

# @method compare
assert(1.0 > 0.0, "1 > 0")
assert(1.0 == 1.0, " 1 = 1")
assert(-1.0 < -0.5, "-1 < -0.5")

# @method abs
assert(-1.0.abs() == 1.0, "|-1| = 1")
assert(1.0.abs() == 1.0, "|1| = 1")

# @method toString
assert(0.0.toString() == "0.000000", "lots of zeros")
assert(-1.0.toString() == "-1.000000", "works with negative numbers too")

# @method toInt
assert(0.0.toInteger() == 0, "0.0 == 0")
assert(1.2.toInteger() == 1, "1.2 gets truncated to 1")

# @method toBool
assert(1.0.toBool(), "non-zero float is true")
assert(!0.0.toBool(), "0 is false")

# @method sqrt
assert(1.0.sqrt() == 1.0, "sqrt(1) = 1")
assert(4.0.sqrt() == 2.0, "sqrt(4) = 2")
