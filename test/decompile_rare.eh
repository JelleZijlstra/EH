#!/usr/bin/ehi
# Test the decompiler with some language features I don't like to use very much

private testf = func: arg
	for 5
		echo 3
	end

	match arg
		case 42; 3
		case _; 5
	end

	$bfind --test --test2=42 -asdf -sjkl=3
end

echo(testf.decompile())
