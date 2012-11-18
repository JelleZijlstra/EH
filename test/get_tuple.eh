#!/usr/bin/ehi
printvar((1, 2, 3)->2)
# Failed before because of an off-by-one error in argument checking
printvar((1, 2, 3)->3)
