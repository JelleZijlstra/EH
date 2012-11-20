#!/usr/bin/ehi
include '../lib/library.eh'

printvar "test\n"
assert("test\n" == "test
")
IO.stderr.puts "test\n"
