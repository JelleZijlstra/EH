#!/usr/bin/ehi
include '../lib/library.eh'
include '../lib/fixed_array_sort.eh'

assert argc == 4, "Usage: " + argv->0 + " min max step"

const (), min, max, step = argv.map func: input
	try
		input.toInt()
	catch
		0
	end
end

sortFunctions = ("bucket sort", FixedArray.bucketSort)::("mergesort", FixedArray.mergeSort)::("quicksort", FixedArray.quickSort)::Nil

fillFunction = i => Random.rand()

private size = min
# here a standard for loop would indeed have been useful
while size <= max
	for (name, f) in sortFunctions
		fa = FixedArray.fill min, fillFunction

		echo name

		#echo f.decompile()
		#echo fa
		(f.bindTo fa)()
		#echo fa
		assert (fa.isSorted()), "this sort doesn't work"
	end
	size += step
end
