#!/usr/bin/ehi
# Illustrate the ability to use anonymous functions within a `given' block.
func gettype: input
	echo given input
		case func: input; ret input.isA Integer; end
			"int"
		case func: input; ret input.isA String; end
			"string"
		case func: input; ret input.isA Bool; end
			"bool"
		case func: input; ret input.isA Range; end
			"range"
		case func: input; ret input.isA Float; end
			"float"
		case func: input; ret input.isA Array; end
			"array"
		case func: input; ret input.isA Object; end
			"object"
		case func: input; ret input.isA Function; end
			"func"
	end
end
# bool
gettype true
# string
gettype "eggs"
# range
gettype 67..68
