#!/usr/bin/ehi
include '../lib/exception.eh'

rescue () => (foo->2 = 0)
rescue () => (echo foo->3)
rescue () => (foo->bar = 0)
rescue () => (echo foo->baz)
