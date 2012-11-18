#!/usr/bin/ehi
include '../lib/brainf.eh'

if argc != 2
	echo('Usage: ' + argv->0 + ' infile')
	ret 1
end
BrainInterpreter.runWithFile(argv->1)
