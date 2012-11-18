#!/usr/bin/ehi
echo 2
func foo:
	ret 1
end
bar = 2
foo(() &bar)
