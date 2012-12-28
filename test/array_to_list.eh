#!/usr/bin/ehi

# Convert an array to a list
include '../lib/library.eh'

arr = ['foo' => {}, 3 => 4]
l = arr.toList()
echo l
printvar l
