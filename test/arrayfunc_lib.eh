#!/usr/bin/ehi
# Functional programming in ehi. Kind of.
func arrayfunc: arrayv, funcv
	for i in arrayv.length()
		arrayv->i = funcv(arrayv->i)
	end
	ret arrayv
end
