#!/usr/bin/ehi
# Functional programming in ehi. Kind of.
func arrayfunc: arrayv, funcv
	for (count $arrayv) count i
		arrayv->$i := $funcv: $arrayv->$i
	end
	ret $arrayv
end
