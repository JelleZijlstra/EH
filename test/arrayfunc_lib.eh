#!/usr/bin/ehi
# Functional programming in ehi. Kind of.
func arrayfunc: arrayv, funcv
	for (arrayv.length ()) count i
		arrayv->i = funcv arrayv->i
	end
	ret arrayv
end
