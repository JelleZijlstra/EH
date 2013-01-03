#!/usr/bin/ehi

private macro = code => (() => code.execute())

private f = (macro with 3)

printvar(f())
