#!/usr/bin/ehi

include '../lib/library.eh'

# test hash literals
assert({}.isA Hash, "empty hash literal")

private h1 = {foo: 'bar', baz: 'quux'}
assert(h1.isA Hash, "non-empty hash literal")

# @method toArray
assert(h1.toArray() == ["foo" => "bar", "baz" => "quux"], "non-empty hash")
assert({}.toArray() == [], "empty hash")

# @method operator->
assert(h1->'foo' == 'bar', "put there in hash literal")
assert(h1->'no such key' == null, "key does not exist")

# @method operator->=
assert((h1.operator->=('new key', 'hello') == 'hello'), "operator->= should return value")
assert(h1->'new key' == 'hello', "hash was changed")
h1->'baz' = 'quux_foo'
assert(h1->'baz' == 'quux_foo', "hash was changed again")

# @method has
assert(h1.has 'baz', "key exists")
assert(!(h1.has 'no such key'), "key does not exist")

# @method delete
assert(h1.delete 'new key' == h1, "delete should return itself")
assert(!(h1.has 'new key'), "key was deleted")

# @method keys
assert(h1.keys() == ['baz', 'foo'], "these keys currently exist")
assert({}.keys() == [], "no keys")

# @method compare
assert({}.compare {} == 0, "empty hashes are equal")
assert({}.compare h1 == -1, "shorter hash comes later")
assert(h1.compare {} == 1, "shorter hash comes later")
assert({foo: 'foo'}.compare {bar: 'foo'} > 0, "key comes later")
assert({foo: 'bar'}.compare {foo: 'foo'} < 0, "value comes earlier")

# @method length
assert({}.length() == 0, "empty hash is empty")
assert(h1.length() == 2, "two elements in h1")

# @method getIterator
private hi = h1.getIterator()
assert(hi.isA(Hash.Iterator), "hashes are iterated over by a Hash.Iterator")

assert(hi.hasNext(), "hash has an element in it")
assert(!({}.getIterator().hasNext()), "iterator is already depleted")

