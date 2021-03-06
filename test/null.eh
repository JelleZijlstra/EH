#!/usr/bin/ehi
include '../lib/exception.eh'

# Illustrates the use of null variables and the printvar library function
bar = null
echo bar
foo = ['foo', null, 'test']
echo(foo -> 1)
# Casting null to a string gives an empty string
baz = bar.toString()
echo baz
# Casting null to an int gives 0 and throws an error
rescue(() => (ban = bar.toInteger()))
rescue(() => printvar ban)
printvar foo
