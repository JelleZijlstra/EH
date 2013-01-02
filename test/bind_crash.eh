#!/usr/bin/ehi

fa = FixedArray.new 5
fi = File.new "File.eh"
f = fi.gets.bindTo fa
f()
