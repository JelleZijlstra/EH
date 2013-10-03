#!/usr/bin/ehi
# Emulate some Haskell niceties

# can't use $ itself like in Haskell, because it is taken by commands
Object##operator$$ arg = this arg

# equivalent to Haskell's .
# This could be done much more nicely if this-bindings were not lost as easily
Object##operator.* g = do
	f = this
	x => f $$ g x
end
# ++ is taken
Object##operator+++ r = this.concat r


# tests
include '../lib/library.eh'

private len x = x.length()
private x2 x = x.map(elt => 2 * elt)
private l = 1::3::Nil
private mapl = len .* x2
echo $$ mapl l
