#!/usr/bin/ehi
# Illustrate functions
func test: n
	$echo n
	$echo 'hi'
	ret n
end

test: 1

test: 2

b = test: 3

$echo b
