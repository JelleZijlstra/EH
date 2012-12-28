#!/usr/bin/ehi
# This program previously ran into a GC bug that caused ehi to crash with an assertion failure.

echo (GC = GarbageCollector)
echo(GC.run())
echo(Map.new())
echo(GC.run())
#echo GC.run()
#echo GC.run()
Map.new()
GC.run()
String.length()
