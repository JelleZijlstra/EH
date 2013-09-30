#!/usr/bin/ehi

# Functions used by Iterable
String##empty() = ''

String##add = String.operator+

String##toString() = this

String##replaceCharacter = func: needle, replacement
	private out = ''
	for char in this
		if char == needle
			out += replacement
		else
			out += char
		end
	end
	out
end

# test that the builtin EH regex does indeed work
String.isRegexAvailable() = "hello".doesMatch ""
