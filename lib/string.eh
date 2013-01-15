#!/usr/bin/ehi

# Functions used by Iterable
String.empty = () => ''

String.add = String.operator+

String.toString = () => this

String.replace = func: needle, replacement
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
