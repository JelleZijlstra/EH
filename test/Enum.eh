#!/usr/bin/ehi

include '../lib/library.eh'

enum A
	B, C(D), E(F, G)
end

enum B
	C
end

assert(A.isA Enum, "it should be")
assert(A.B.isA(Enum.Member), "member class")
assert(A.C(3).isA(Enum.Instance), "instance class")

# @method new
assertThrows((() => Enum.new()), TypeError, "Directly creating a new Enum is not allowed")
assertThrows((() => A.new()), TypeError, "It doesn't work with an instance either")

# @method size
assert(A.size() == 3, "A has 3 members")
assert(B.size() == 1, "B has 1 member")

# @method toString
assert(A.toString() == "enum A
	E(F, G), C(D), B
end", "But the members are reversed")
assert(B.toString() == "enum B
	C
end", "only one member")
