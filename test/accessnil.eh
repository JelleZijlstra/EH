#!/usr/bin/ehi
include '../lib/exception.eh'

rescue func: -> (foo->2 = 0)
rescue func: -> (echo foo->3)
rescue func: -> (foo->bar = 0)
rescue func: -> (echo foo->baz)
