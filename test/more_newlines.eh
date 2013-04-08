#!/usr/bin/ehi

# Test newlines within hash and array literals and parentheses
printvar {
	foo: 'bar', 'hello': 'world'}

printvar [
	'bar', 'world', 3]

echo (
	1 + 1)
