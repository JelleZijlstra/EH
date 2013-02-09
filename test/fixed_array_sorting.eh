#!/usr/bin/ehi
include '../lib/library.eh'
include '../lib/fixed_array_sort.eh'

l = 0::3::2::1::Nil

l = 20.toList()

fa = FixedArray.mapFrom l
fa.bubbleSort()
assert (fa.isSorted(), "bubble sort doesn't work")
fa = FixedArray.mapFrom l
fa.insertionSort()
assert (fa.isSorted(), "insertion sort doesn't work")
fa = FixedArray.mapFrom l
fa.selectionSort()
assert (fa.isSorted(), "selection sort doesn't work")
fa = FixedArray.mapFrom l
fa.combSort()
assert (fa.isSorted(), "comb sort doesn't work")
fa = FixedArray.mapFrom l
fa.mergeSort()
assert (fa.isSorted(), "mergesort doesn't work")
fa = FixedArray.mapFrom l
fa.quickSort()
assert (fa.isSorted(), "quicksort doesn't work")
fa = FixedArray.mapFrom l
fa.bucketSort()
assert (fa.isSorted(), "bucket sort doesn't work")
fa = FixedArray.mapFrom l
fa.stoogeSort()
assert (fa.isSorted(), "stooge sort doesn't work")
