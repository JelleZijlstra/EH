#!/usr/bin/ehi
# Even shorter short functions

f = x => x
echo(f 3)
echo(f.decompile())
f2 = a, b => a + b
echo f2(1, 2)
