#!/usr/bin/ehi

include '../lib/library.eh'

m = Map.new()

m->0 = 3
echo m->0

m.each printvar
