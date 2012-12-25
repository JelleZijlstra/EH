#!/usr/bin/ehi

enum List
	Nil, Cons(head, tail)

	public map = f => match this
		case Nil; Nil
		case Cons(@hd, @tl); Cons(f hd, tl.map f)
	end

	public reduce = f, b => match this
		case Nil; b
		case Cons(@hd, @tl); f(hd, tl.reduce(f, b))
	end

	public toString = () => this.reduce((elt, b => elt.toString() + "::" + b), "Nil")
end

Object.operator:: = rhs => List.Cons(this, rhs)
l = 3::4::(List.Nil)
echo l
echo(l.map(2.operator*))
