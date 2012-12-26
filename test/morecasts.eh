#!/usr/bin/ehi
include '../lib/exception.eh'

# More crazy casts

printvar((1..3).toString())
printvar(File.new().toBool())
rescue(() => printvar((n => n).toBool()))
rescue(() => printvar(3.14.toRange()))
printvar(null.toArray())
