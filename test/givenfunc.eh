#!/usr/bin/ehi
# Illustrate the ability to use anonymous functions within a `given' block.
func gettype: input
	echo given input
		case func: input; ret is_int input; end
			"int"
		case func: input; ret is_string input; end
			"string"
		case func: input; ret is_bool input; end
			"bool"
		case func: input; ret is_range input; end
			"range"
		case func: input; ret is_float input; end
			"float"
		case func: input; ret is_array input; end
			"array"
		case func: input; ret is_object input; end
			"object"
		case func: input; ret is_func input; end
			"func"
	end
end
# bool
gettype true
# string
gettype "eggs"
# range
gettype 67..68
