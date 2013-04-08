#!/usr/bin/ehi
# Array testing
include '../lib/library.eh'

private emptyArray = []
private numericArray = ['foo', 'bar']
private stringArray = ['foo', 42]

private arrayArray = [emptyArray, numericArray, stringArray]

private typeChecker = arr => assert(arr.isA Array, "Should be an array")

typeChecker arrayArray
arrayArray.each typeChecker

# @method initialize
private fromRange = Array.new(1..3)
typeChecker fromRange

# @method length
private lengthChecker(arr, length) = assert(arr.length() == length, "Length of this array should be " + length)
lengthChecker(emptyArray, 0)
lengthChecker(numericArray, 2)
lengthChecker(stringArray, 2)

# @method has
assert(!(emptyArray.has 0), "array is empty")
assert(numericArray.has 0, "key 0 should exist")
assert(numericArray.has 1, "key 1 should exist")
assert(!(numericArray.has 2), "key 2 should not exist")
assert(stringArray.has 1, "key 1 should exist")
assertThrows((() => stringArray.has(1..5)), TypeError, "invalid key type")

# @method operator->
assertThrows((() => emptyArray->3), ArgumentError, "key does not exist")
assert(numericArray->1 == 'bar', "second value is bar")
assert(stringArray->0 == 'foo', "also works with string keys")
assertThrows((() => stringArray->(1..5)), TypeError, "operator-> throws on invalid type")

# @method push
numericArray.push 42
assert(numericArray->2 == 42, "just added 42")
assert(numericArray.length() == 3, "length is now 3")

# @method operator->=
private setChecker = func: arr, key, value
	assert((arr->key = value) == value, "operator->= should return the rvalue")
	assert(arr.has key, "operator->= should set")
	assert(arr->key == value, "operator->= should set correctly")
end
setChecker(numericArray, 2, "hello")
setChecker(numericArray, 1, {})
setChecker(numericArray, 0, 'baz')
assertThrows((() => stringArray->(1..5) == 3), TypeError, "operator->= throws on invalid type")

# @method toArray
private toArrayChecker arr = assert(arr.toArray() == arr, "toArray shouldn't do anything")
arrayArray.each toArrayChecker

# @method toTuple
assert(numericArray.toTuple().sort() == ('baz', 'hello', {}).sort(), "tuple conversion")
assert(stringArray.toTuple().sort() == ('foo', 42).sort(), "tuple conversion")

# @method getIterator
private it = numericArray.getIterator()
assert(it.isA(Array.Iterator), "Arrays are iterated over by an Array.Iterator")

# Iterable.toList uses getIterator() internally
assert(numericArray.toList().sort() == ('baz':: 'hello'::{}::Nil).sort(), "must be the same")

# @method compare
# Note that methods like < call compare internally

# test < and >, which should always be reversed from each other
private gt_lt = func: lhs, rhs, desc
	assert(lhs < rhs, desc)
	assert(rhs > lhs, "reverse of: " + desc)
end

gt_lt(['foo'], ['foo', 'bar'], "shorter array is less")
gt_lt(['bar'], ['foo'], "smaller value")
gt_lt(['bar', 'foo', 'baq'], ['bar', 'foo', 'baz'], "slightly larger arrays")
gt_lt(['bar'], ['baz'], "string value comparison")
gt_lt(['bar', 'foo'], ['foo', 'bar'], "not equal")

assert([] == [], "empty arrays are equal")
assert([] != [1], "arrays are different")
assert(['foo', 'bar'] == ['foo', 'bar'], "still the same")
