#!/usr/bin/ehi
# I thought this might fail, but it seems to work.
c1 = CountClass.new ()
c2 = CountClass.new ()
f = File.new ()
# 1, 0
c1.docount ()
# 2, 0
c1.docount ()
# 3, 3
c2.setcount (c1.docount ())

# 4
echo(c1.docount ())
# 4
echo(c2.docount ())
c2.setcount 0
echo(c2.docount ())
