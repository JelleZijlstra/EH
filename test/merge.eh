#!/usr/bin/ehi
include '../lib/library.eh'

merge lst1 lst2 = match lst1, lst2
	case _, Nil; lst1
	case Nil, _; lst2
	case Cons(@hd1, @tl1), Cons(@hd2, @tl2) when hd1 < hd2; hd1::(merge tl1 lst2)
	case Cons(@hd1, @tl1), Cons(@hd2, @tl2); hd2::(merge lst1 tl2)
	case _; printvar(lst1, lst2)
end

split lst l r = match lst
	case Nil; l, r
	case Cons(@hd, @tl); split tl (hd::r) l
end

sort lst = match lst
	case Nil | Cons(_, Nil); lst
	case _
		private l, r = split lst Nil Nil
		merge (sort l) (sort r)
end

l = List.makeRandom 10
sorted = sort l
echo l
echo sorted
assert(sorted.isSorted())
