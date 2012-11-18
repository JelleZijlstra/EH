#!/usr/bin/ehi
hash = { 'foo': 3, 'bar': 'test' }
printvar hash
printvar(hash.has 'foo')
printvar(hash->'foo')
hash->'baz' = 42.0
printvar(hash->'baz')
