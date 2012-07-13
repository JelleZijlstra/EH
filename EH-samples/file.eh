#!/usr/bin/ehi
# Illustrates the File class
f = new File
$f->open: "file.eh.helper"
c = false
while $c !== null
	c = $f->getc:
	echo $c
end
$f->close:
