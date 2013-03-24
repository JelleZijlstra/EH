#!/usr/bin/ehi
# Emulate some Haskell niceties

# can't use $ itself like in Haskell, because it is taken by commands
Object.operator$$ arg = this arg

# equivalent to Haskell's .
# This could be done much more nicely if this-bindings were not lost as easily
Object.operator.* = func: g
	f = this
	x => f $$ g x
end

# ++ is taken
Object.operator+++ r = this.concat r
