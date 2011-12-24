// Implementation of the McCarthy function in EH
func mccarthy: n
	if $n > 100
		$ ret = $n - 10
		ret $ret
	else
		$ arg = $n + 11
		$ arg = mccarthy: $arg
		$ arg = mccarthy: $arg
		ret $arg
	endif
endfunc

$ input = 5
$ res = mccarthy: $input
echo $res
