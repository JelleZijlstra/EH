#!/usr/bin/ehi
include '../lib/library.eh'
include '../lib/fixed_array_sort.eh'

l = 0::3::2::1::Nil

fa = FixedArray.with l
fa.bubbleSort()
assert (fa.isSorted(), "bubble sort doesn't work")
fa = FixedArray.with l
fa.insertionSort()
assert (fa.isSorted(), "insertion sort doesn't work")
fa = FixedArray.with l
fa.selectionSort()
assert (fa.isSorted(), "selection sort doesn't work")
fa = FixedArray.with l
fa.combSort()
assert (fa.isSorted(), "comb sort doesn't work")
fa = FixedArray.with l
fa.mergeSort()
assert (fa.isSorted(), "mergesort doesn't work")
fa = FixedArray.with l
fa.quickSort()
assert (fa.isSorted(), "quicksort doesn't work")
fa = FixedArray.with l
fa.bucketSort()
assert (fa.isSorted(), "bucket sort doesn't work")
