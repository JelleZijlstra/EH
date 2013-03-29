#!/usr/bin/ehi
include '../lib/library.eh'

assert("foo".any("o".operator==))
assert("foo".all(c => c == 'f' || c == 'o'))
assert("foo".containsMember 'f')
assert(!("foo".all('f'.operator==)))
