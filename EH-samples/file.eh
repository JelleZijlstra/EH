#!/usr/bin/ehi
# Illustrates the File class
$ f = new File
$f.open: "file.eh.helper"
$ c = null
while $c !== -1
	$ c = $f.getc:
	echo $c
end
$f.close:
