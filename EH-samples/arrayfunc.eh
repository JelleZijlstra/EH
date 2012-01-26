#!/usr/bin/ehi
# Functional programming in ehi. Kind of.
func arrayfunc: arrayv, funcv
	for (count $arrayv) count i
		set arrayv->$i = $funcv: $arrayv->$i
	end
	ret $arrayv
end

printvar: arrayfunc: [1, 2, 3, 4], func: n; ret 2 * $n; end
printvar: arrayfunc: ["foo", "bar", "baz"], func: str; ret $str->0; end
