#!/usr/bin/ehi
include '../lib/library.eh'

butlast lst = match lst
	case Cons(@hd, Cons(_, Nil)); hd
	case Cons(_, Cons(_, _) as tl); butlast tl
end

printvar (butlast (1::3::Nil))
printvar (butlast (2::1::3::Nil))
