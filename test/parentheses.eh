#!/usr/bin/ehi
# Stuff with parentheses
# Should print 9
echo (2 + 1) * 2 + 3
# Should print 7
echo 2 + 1 * 2 + 3
# Should print 7
echo 2 + (1 * 2) + 3
# Should print 3
echo [ 1 * 2, 3, 1 + 2] -> 1
# Should print 1 (2 > 2 is false)
echo 1 + 2 < 2
# Should print true
echo 2 < (2 + 1)
