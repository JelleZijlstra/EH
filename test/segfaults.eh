#!/usr/bin/ehi
include '../lib/exception.eh'

# Stuff that has tended to cause segfaults
echo 'Test 1'
rescue func: -> (echo 'data'->2010)

echo 'Test 2'
rescue func: -> (data == data)
rescue func: -> (data->2010 = 4)

echo 'Test 3'
a = 42
rescue func: -> (a->(data->2010) = 5)

echo 'Test 4'
rescue func: -> (echo data->('data'->2010))

echo 'Test 5'
arr = []
rescue func: -> (arr->(data->5) = 5)

echo 'Test 6'
rescue func:
	for (a, b) in 'data'->2010 {}
end
