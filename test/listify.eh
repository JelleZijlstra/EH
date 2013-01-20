#!/usr/bin/ehi

include '../lib/library.eh'
include '../lib/preprocessor.eh'

private process = code => echo(Preprocessor.listify(code->0))

private code = raw (1, 2, 3)

process raw (1, 2, 3)

process raw if 3
	4
elsif 5
	6
elsif 7
	8
elsif 9
	10
else
	11
end
