#!/usr/bin/ehi
# Test the decompiler with some language features I don't like to use very much

private testf = func: arg
	for 5
		echo 3
	end

	switch arg
		case 42; 3
		default; 5
	end

	$bfind --test --test2=42 -asdf -sjkl=3
end

echo(testf.decompile())
