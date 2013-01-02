#!/usr/bin/ehi
include '../lib/library.eh'

assertThrows((() => throw(Exception.new "hello")), Exception, "should throw an Exception")

# @method initialize
private e = Exception.new("hello")
assert(e.isA Exception)

# @method toString
assert(e.toString() == "hello", "message should appear in toString")
