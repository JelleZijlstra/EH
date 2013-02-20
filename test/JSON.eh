#!/usr/bin/ehi

include '../lib/library.eh'

# Test the JSON parser, and in the process demonstrate that EH supports JSON syntax pretty well

# @method parse

# numeric literals
assert(JSON.parse "3" == 3)
assert(JSON.parse "-3" == -3)
assert(JSON.parse "0" == 0)
assert(JSON.parse "42" == 42)
assert(JSON.parse "1.0" == 1.0)
assert(JSON.parse "-0.4" == -0.4)
assert(JSON.parse "42.4" == 42.4)

# constants
assert(JSON.parse "null" == null)
assert(JSON.parse "true" == true)
assert(JSON.parse "false" == false)

# string literals
assert(JSON.parse '"foo"' == "foo", "simple string")
assert(JSON.parse '"foo\\"bar"' == 'foo"bar', "escaped double quote")
assert(JSON.parse '"foo\\nbar"' == 'foo
bar', "escaped newline")
assert(JSON.parse '"foo\\tbar"' == "foo	bar", "escaped tab")
assert(JSON.parse '"foo\\\\bar"' == 'foo\\bar', "escaped backslash")

# objects
assert(JSON.parse '{}' == {}, "empty hash")
assert(JSON.parse '{"foo": "bar"}' == {foo: "bar"}, "hash with one member")
assert(JSON.parse '{"foo": "bar", "baz": 3}' == {foo: "bar", baz: 3}, "hash with two members")

# arrays
assert(JSON.parse '[]' == [], "empty array")
assert(JSON.parse '[0]' == [0], "array with one member")
assert(JSON.parse '[0, 1, "foo"]' == [0, 1, "foo"], "array with two members")
