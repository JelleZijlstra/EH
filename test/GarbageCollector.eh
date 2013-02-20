#!/usr/bin/ehi

include '../lib/library.eh'

# @method run
# Not much to assert here, except that it doesn't break the program.
GarbageCollector.run()

# @method stats
# Should print data
GarbageCollector.stats()
