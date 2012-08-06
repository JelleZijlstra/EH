#!/usr/bin/ehi
include '../lib/exception.eh'

# I'm afraid this will crash ehi
Integer.length = String.length
rescue func: -> (Hash.has "foo")
4.length ()
