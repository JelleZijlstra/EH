#!/usr/bin/ehi
# This should now also be possible
f = func: {
  foo = []
  for 5 count i {
    foo->i = i
  }
  foo
}
printvar f()
