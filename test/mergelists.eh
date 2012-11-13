#!/usr/bin/ehi
include '../lib/library.eh'

# split a list into two equal sublists
public split = func: list, l, r
	if list.isEmpty()
		l, r
	else
		hd, tl = list
		split(tl, hd::r, l)
	end
end

# merge two sorted lists
public merge = func: l, r
	if l.isEmpty()
		r
	elsif r.isEmpty()
		l
	else
		lhd, ltl = l
		rhd, rtl = r
		given lhd <=> rhd
			case -1; lhd::merge(ltl, r)
			case 0; lhd::merge(ltl, rtl)
			case 1; rhd::merge(l, rtl)
		end
	end
end

echo merge (3::Nil, 4::Nil)

# merge a list of sorted lists into a single sorted list
public merge_lists = func: lists
	if lists.isEmpty()
		Nil
	elsif lists.isSingleton()
		lists->0
	else
		l, r = split(lists, Nil, Nil)
		merge((merge_lists l), merge_lists r)
	end
end

echo merge_lists (1::2::Nil)::(3::4::Nil)::Nil
