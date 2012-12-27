#!/usr/bin/ehi
include '../lib/library.eh'

# Demonstrates all features of EH pattern matching

# In the standard library, List is defined as follows:

# enum List
# 	Nil, Cons(head, tail)
# end

List.prototype.zip = f, rhs => match this, rhs
	# pattern matches if both operands are Nil
	case Nil, Nil; Nil
	# pattern matches if either operand is Nil; second operand is ignored
	case (_, Nil) | (Nil, _); throw(ArgumentError.new("Lists are not of equal length", "List.zip", (this, rhs)))
	# default case: process the heads of the lists and recurse on the tails
	case Cons(@lhd, @ltl), Cons(@rhd, @rtl); Cons(f(lhd, rhd), ltl.zip(f, rtl))
	# if either of the elements is not a list (and neither is Nil), no case will match and an error will be thrown
end

# Create two lists and sum their elements one by one
l1 = 1::2::Nil
l2 = 3::4::Nil
echo(l1.zip((l, r => l + r), l2))
