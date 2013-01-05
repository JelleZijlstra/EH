#!/usr/bin/ehi

private macro = code, context => (() => code.execute())

private f = (macro raw 3)

printvar(f())
