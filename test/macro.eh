#!/usr/bin/ehi

private macro = code, context => (() => code.execute context)

private f = (macro raw 3)

printvar(f())
