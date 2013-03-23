#!/usr/bin/ehi
include '../lib/library.eh'

private f = File.new "File.eh"

# @method initialize
private noFile = File.new()
assert(!(noFile.isOpen()), "no file opened")
private withFile = File.new "File.eh"
assert(withFile.isOpen(), "file was opened")

# @method open
assert(noFile.open "File.eh", "should be able to open this file")
assert(noFile.isOpen(), "should be open now")
assert(!(noFile.open "/path/does/not/exist/on/any/system/where/anyone/will/ever/test/this"), "should fail")
assert(!(noFile.isOpen()), "open closes the currently open file before trying to open the new one")

# @method getc
assert(withFile.getc() == '#', "First character in this file is #")
assert(withFile.getc() == '!', "Second character is !")
assert(noFile.getc() == null, "getc returns null on failure")

# @method gets
assert(withFile.gets() == "/usr/bin/ehi\n", "The rest of the line")
withFile.close()

# @method puts
shell "touch File.eh.tmp"
private writeFile = File.new "File.eh.tmp"
assert(writeFile.isOpen(), "file should be open")
writeFile.puts "Hello, world!"
writeFile.close()

# @method readFile
assert(File.readFile "File.eh.tmp" == "Hello, world!", "just wrote that")
shell "rm File.eh.tmp"

# @method close
assert(noFile.open "File.eh", "should be able to open this file again")
assert(noFile.isOpen(), "file is open")
noFile.close()
assert(!(noFile.isOpen()), "now it isn't")

# @method finalize
private anotherFile = File.new "File.eh"
assert(anotherFile.isOpen(), "another file is open")
anotherFile.finalize()
assert(!(anotherFile.isOpen()), "now closed")

# @method isOpen
assert(!(anotherFile.isOpen()), "now closed")
assert(anotherFile.open "File.eh", "open it again")
assert(anotherFile.isOpen(), "now open")
anotherFile.close()
