#!/usr/bin/ehi
include '../lib/library.eh'
include '../lib/fixed_array_sort.eh'

l = 0::3::2::1::Nil
echo l

l = 20.toList()
echo l

fa = FixedArray.mapFrom l
fa.bubbleSort()
assert (fa.isSorted(), "bubble sort doesn't work")
echo fa
fa = FixedArray.mapFrom l
fa.insertionSort()
assert (fa.isSorted(), "insertion sort doesn't work")
echo "insertion"
fa = FixedArray.mapFrom l
fa.selectionSort()
assert (fa.isSorted(), "selection sort doesn't work")
echo "selection"
fa = FixedArray.mapFrom l
fa.combSort()
assert (fa.isSorted(), "comb sort doesn't work")
echo "comb"

fa = FixedArray.mapFrom l
fa.mergeSort()
assert (fa.isSorted(), "mergesort doesn't work")
echo "merge"

fa = FixedArray.mapFrom l
echo 'created'
fa.quickSort()
assert (fa.isSorted(), "quicksort doesn't work")
echo "quick"

fa = FixedArray.mapFrom l
fa.bucketSort()
assert (fa.isSorted(), "bucket sort doesn't work")
echo "bucket"

fa = FixedArray.mapFrom l
fa.stoogeSort()
assert (fa.isSorted(), "stooge sort doesn't work")
echo "stooge"

fa = FixedArray.mapFrom l
fa.heapSort()
assert (fa.isSorted(), "heap sort doesn't work")
