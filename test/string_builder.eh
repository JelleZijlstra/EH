#!/usr/bin/ehi

private sb = String.Builder.new()

sb << 42
sb << "hello"

echo sb

private sb2 = String.Builder.new()
sb2 << true
sb2 << 3 << "hey" << []

echo sb2
printvar sb2

echo sb
sb << sb2

echo sb
