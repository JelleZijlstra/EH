#!/usr/bin/ehi

private str = "Hello, world!"

echo(str.doesMatch "^H")
echo(str.doesMatch "hello")
echo(str.replace("^Hello", "Goodbye"))
