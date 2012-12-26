#!/usr/bin/ehi
include '../lib/library.eh'

List.prototype.zip = f, rhs => match this, rhs
	case Nil, Nil; Nil
	case (_, Nil) | (Nil, _); throw(ArgumentError.new("Lists are not of equal length", "List.zip", (this, rhs)))
	case Cons(@lhd, @ltl), Cons(@rhd, @rtl); Cons(f(lhd, rhd), ltl.zip(f, rtl))
end

l1 = 1::2::Nil
l2 = 3::4::Nil
echo(l1.zip((l, r => l + r), l2))
