#!/usr/bin/ehi

# This crashes for some reason
GC = GarbageCollector

this.type().inherit String
GC.stats()
try
	length 0
catch
end
EH.collectGarbage()
try
	length 0
catch
end
EH.collectGarbage()
echo 3
#printvar global
echo 3
GC.stats()
GC.stats()
printvar (c = Map.new())
printvar(c.has())
operator-> 0
