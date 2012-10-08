#!/usr/bin/ehi
include '../lib/library.eh'

fa = FixedArray.new 3
assert (fa.size()) == 3, "test the FixedArray.size() method"
assert fa->0 == null, "members of FixedArray must be initialized to null"
fa->0 = 3
assert fa->0 == 3, "test whether I can set a FixedArray member"
for i in fa.size()
	fa->i = i
end
for i in fa.size()
	assert fa->i == i, "test whether I can set all members of a FixedArray"
end
fa.each i => echo i
