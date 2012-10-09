#!/usr/bin/ehi
include '../lib/library.eh'
include '../lib/fixed_array_sort.eh'

fa = FixedArray.with 0::3::2::1::Nil
fa.bubbleSort()
assert (fa.isSorted()), "bubble sort doesn't work"
fa = FixedArray.with 0::3::2::1::Nil
fa.insertionSort()
assert (fa.isSorted()), "insertion sort doesn't work"
fa = FixedArray.with 0::3::2::1::Nil
fa.selectionSort()
assert (fa.isSorted()), "selection sort doesn't work"
fa = FixedArray.with 0::3::2::1::Nil
fa.combSort()
assert (fa.isSorted()), "comb sort doesn't work"
fa = FixedArray.with 0::3::2::1::Nil
fa.mergeSort()
assert (fa.isSorted()), "mergesort doesn't work"
fa = FixedArray.with 0::3::2::1::Nil
fa.quickSort()
assert (fa.isSorted()), "quicksort doesn't work"
fa = FixedArray.with 0::3::2::1::Nil
fa.bucketSort()
assert (fa.isSorted()), "bucket sort doesn't work"
