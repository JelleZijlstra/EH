#!/usr/bin/ehi
a, (b, c) = 1, (2, 3)
echo a
echo b
echo c

(a, b), c = ['test', 'foo'], 'bar'
echo a
echo b
echo c

printvar(1, (2, 3))
