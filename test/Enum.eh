#!/usr/bin/ehi

include '../lib/library.eh'

enum A
	B, C(D), E(F, G)
end

enum B
	C
end

assert(A.isA Enum, "it should be")
assert(A.B.isA(Enum), "member class")
assert(A.C(3).isA(Enum), "instance class")

# @method new
assertThrows((() => Enum.new()), TypeError, "Directly creating a new Enum is not allowed")
assertThrows((() => A.new()), TypeError, "It doesn't work with an instance either")

# @method toString
assert(A.toString() == "enum A
	B, C(D), E(F, G)
end", "It's like decompile")
assert(B.toString() == "enum B
	C
end", "only one member")

# @method operator()
assertThrows((() => A.B()), MiscellaneousError, "Cannot instantiate nullary")
assertThrows((() => A.E(3)), TypeError, "Wrong number of arguments")
assertThrows((() => A()), TypeError, "Cannot instantiate class")
assert(A.C(3).type() == "A", "Instantiation")
private instance = A.E(3, 4)
assert(instance.typeId() == A.typeId(), "it is an A")
assert(instance->0 == 3, "First argument")
assert(instance->1 == 4, "Second argument")

# @method compare
assert(Enum < A, "Enum was defined first")
assert(A < B, "A was defined first")
assert(B > A.B, "All A comes first")
assert(A.B > A.C, "B was defined first")
assert(A.C < A.C(3), "Constructors come first")
assert(A.C(3) < A.C(4), "Arguments are compared")

# @method typeId
assert(A.typeId() == A.B.typeId(), "typeId matches for all enum members and instances")
assert(A.B.typeId() == A.C(4).typeId(), "typeId matches (again)")
assert(A.typeId() != B.typeId(), "different enums have a different typeId")

# @method type
assert(A.type() == "A", "type of A is A")
assert(A.B.type() == "A", "type is A")
assert(A.C(3).type() == "A", "type is still A")
assert(B.type() == "B", "type is B")
assert(B.C.type() == "B", "type is B")

# @method isConstructor
assert(!A.B.isConstructor(), "Nullary member is not a constructor")
assert(A.C.isConstructor(), "is a constructor")
assert(!A.C(3).isConstructor(), "not a constructor")

# @method operator->
assertThrows((() => A.B->0), ArgumentError, "Cannot call operator-> on nullary")
assertThrows((() => A.C->0), MiscellaneousError, "Cannot call operator-> on constructor")
assert(A.C(3)->0 == 3, "argument is 3")
assertThrows((() => A.C(3)->(-1)), ArgumentError, "Out of range")
assertThrows((() => A.C(3)->1), ArgumentError, "out of range")
assert(A.E(3, 4)->1 == 4, "argument is 4")

# @method map
assert(instance.map(2.operator*) == A.E(6, 8), "map works")
