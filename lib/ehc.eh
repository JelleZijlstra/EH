#!/usr/bin/ehi

include 'library.eh'
include 'preprocessor.eh'
include 'compiler.eh'

private co = Compiler.new(argv->1)
#private co = Compiler.new("/Users/jellezijlstra/code/EH/lib/tmp/test5.eh")
co.compile "tmp/compile_test.cpp"
