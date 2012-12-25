#!/usr/bin/ehi

enum X
	A(b), B(c), D

	public run = () => match this
		case D; echo "D"
		case A(@e); echo("A of " + e)
		case B(3); echo("B of " + this.c)
		case B(@f); echo("Other B of " + f)
	end
end

X.D.run()
X.A(2).run()
X.B(3).run()
X.B(4).run()
