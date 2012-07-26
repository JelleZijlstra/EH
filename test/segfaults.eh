#!/usr/bin/ehi
# Stuff that has tended to cause segfaults
$echo 'Test 1'
$echo 'data'->2010

$echo 'Test 2'
data == data
data->2010 = 4

$echo 'Test 3'
a = 42
a->(data->2010) = 5

$echo 'Test 4'
$echo data->('data'->2010)

$echo 'Test 5'
arr = []
arr->(data->5) = 5

$echo 'Test 6'
for 'data'->2010 as a => b {}
