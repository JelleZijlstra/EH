#!/usr/bin/ehi
# Array testing
include '../lib/library.eh'

private emptyArray = []
private numericArray = ['foo', 'bar']
private stringArray = ['foo' => 'foo', 42 => 'bar']

private arrayArray = [emptyArray, numericArray, stringArray]

private typeChecker = arr => assert(arr.isA Array, "Should be an array")

typeChecker arrayArray
arrayArray.each(k, v => typeChecker v)

# @method initialize
private fromRange = Array.new(1..3)
typeChecker fromRange

# @method length
private lengthChecker = arr, length => assert(arr.length() == length, "Length of this array should be " + length)
lengthChecker(emptyArray, 0)
lengthChecker(numericArray, 2)
lengthChecker(stringArray, 2)

# @method has
assert(!emptyArray.has "anything", "array is empty")
assert(numericArray.has 0, "key 0 should exist")
assert(numericArray.has 1, "key 1 should exist")
assert(!numericArray.has 2, "key 2 should not exist")
assert(stringArray.has "foo", "key foo should exist")
assert(stringArray.has 42, "key 42 should exist")
assert(!stringArray.has 0, "key 0 should not exist")
assert(!stringArray.has(1..5), "invalid key type")

# @method operator->
assert(emptyArray->3 == null, "key does not exist")
assert(numericArray->1 == 'bar', "second value is bar")
assert(stringArray->'foo' == 'foo', "also works with string keys")
assertThrows((() => stringArray->(1..5)), TypeError, "operator-> throws on invalid type")

# @method operator->=
private setChecker = func: arr, key, value
	assert((arr->key = value) == value, "operator->= should return the rvalue")
	assert(arr.has key, "operator->= should set")
	assert(arr->key == value, "operator->= should set correctly")
end
setChecker(numericArray, 4, "hello")
setChecker(numericArray, "foo", {})
setChecker(numericArray, 0, 'baz')
assertThrows((() => stringArray->(1..5) == 3), TypeError, "operator->= throws on invalid type")

# @method toArray
private toArrayChecker = arr => assert(arr.toArray() == arr, "toArray shouldn't do anything")
arrayArray.each(k, v => toArrayChecker v)

# @method toTuple
assert(numericArray.toTuple().sort() == ('baz', 'bar', 'hello', {}).sort(), "tuple conversion")
assert(stringArray.toTuple().sort() == ('foo', 'bar').sort(), "tuple conversion")

# @method getIterator
private it = numericArray.getIterator()
assert(it.isA(Array.Iterator), "Arrays are iterated over by an Array.Iterator")

# Iterable.toList uses getIterator() internally
assert(numericArray.toList().sort() == ((0, 'baz')::(1, 'bar')::(4, 'hello')::('foo', {})::Nil).sort(), "must be the same")

# @method compare
# Note that methods like < call compare internally

# test < and >, which should always be reversed from each other
private gt_lt = func: lhs, rhs, desc
	assert(lhs < rhs, desc)
	assert(rhs > lhs, "reverse of: " + desc)
end

gt_lt(['foo'], ['foo', 'bar'], "shorter array is less")
gt_lt(['bar'], ['foo'], "smaller value")
gt_lt(['bar'], [1 => 'bar'], "smaller key")
gt_lt(['bar', 'foo', 'baq'], ['bar', 'foo', 'baz'], "slightly larger arrays")
gt_lt(['foo' => 'bar'], ['koe' => 'baz'], "string key comparison")
gt_lt(['foo' => 'bar'], ['foo' => 'baz'], "string value comparison")
gt_lt(['bar', 'foo'], ['foo', 'bar'], "not equal")

assert([] == [], "empty arrays are equal")
assert([] != [1], "arrays are different")
assert([1 => 'foo', 2 => 'bar'] == [2 => 'bar', 1 => 'foo'], "still the same")
