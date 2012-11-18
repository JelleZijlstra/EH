#!/usr/bin/ehi
class Foo
	operator& = func: rhs
		echo('Binary and with ' + rhs)
	end
	operator| = func: rhs
		echo('Binary or with ' + rhs)
	end
	operator^ = func: rhs
		echo('Binary xor with ' + rhs)
	end
	operator~ = func:
		echo 'Binary negation'
	end
	operator_uminus = func:
		echo 'Unary minus'
	end
	operator! = func:
		echo 'Negation'
	end
	# Not really
	operator* = rhs => (echo 'Unary minus')
end
f = Foo.new ()
f & 2
f ^ 3
f | 4
~f
f * -1
!f
