#!/usr/bin/ehi
# Illustrate the ability to use anonymous functions within a `match' block.
func gettype: input
	echo match input
		case _ when input.isA Integer
			"int"
		case _ when input.isA String
			"string"
		case _ when input.isA Bool
			"bool"
		case _ when input.isA Range
			"range"
		case _ when input.isA Float
			"float"
		case _ when input.isA Array
			"array"
		case _ when input.isA Object
			"object"
		case _ when input.isA Function
			"func"
	end
end
# bool
gettype true
# string
gettype "eggs"
# range
gettype(67..68)
