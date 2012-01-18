#!/usr/bin/ehi
# Illustrate the ability to use anonymous functions within a `given' block.
func gettype: in
	echo given $in
		case func: in; ret is_int: $in; end
			"int"
		case func: in; ret is_string: $in; end
			"string"
		case func: in; ret is_bool: $in; end
			"bool"
		case func: in; ret is_range: $in; end
			"range"
		case func: in; ret is_float: $in; end
			"float"
		case func: in; ret is_array: $in; end
			"array"
		case func: in; ret is_object: $in; end
			"object"
		case func: in; ret is_func: $in; end
			"func"
	end
end
# bool
gettype: true
# string
gettype: "eggs"
# range
gettype: 67..68
