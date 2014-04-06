#!/usr/bin/ehi
include '../lib/library.eh'

f = x => match x
	case false, false; echo x
	case true, false; echo x
	case false, true; echo x
	case true, true; echo x
end

f(true, false)
f(false, true)
f 42
