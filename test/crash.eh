#!/usr/bin/ehi

# This crashes for some reason
GC = GarbageCollector

inherit String
GC.stats()
try {
  length 0
} catch {}
collectGarbage()
try {
  length 0
} catch {}
collectGarbage()
echo 3
#printvar global
echo 3
GC.stats()
GC.stats()
printvar (c = CountClass.new())
printvar c.docount()
operator-> 0
