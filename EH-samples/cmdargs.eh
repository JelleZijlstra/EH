# cmdargs.eh
# Test and illustrate passing of arguments to shell scripts
$ foo = 3
$ baz = &foo
test --ref=$baz
test --arr=[$foo, $baz, 42]
test --'bool'=true
test --num=45
test --marr=[[1, 2], ['foo' => 3], 4]
