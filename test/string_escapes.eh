#!/usr/bin/ehi
include '../lib/library.eh'

# single-quoted strings are raw, except for escaped \'
printvar 'foo\nbar'
printvar 'foo\'bar'
printvar 'foo\\bar'
printvar 'foo
bar'

# double-quoted strings have escape sequences
printvar "foo\nbar"
printvar "foo\tbar"
printvar "foo\\bar"
printvar "foo\"bar"
printvar "foo\rbar"

try_str str = rescue(() => EH.eval(str + ";"))
try_str '"foo\xbar"'
try_str '"foo\u0309bar"'
