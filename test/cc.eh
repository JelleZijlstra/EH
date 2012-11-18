#!/usr/bin/ehi
# This program previously ran into a GC bug that caused ehi to crash with an assertion failure.

echo (GC = GarbageCollector)
echo(GC.run())
echo(CountClass.new())
echo(GC.run())
#echo GC.run()
#echo GC.run()
CountClass.new()
GC.run()
String.length()
