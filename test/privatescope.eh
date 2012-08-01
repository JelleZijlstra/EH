#!/usr/bin/ehi
# Test private scope (mostly because I'm curious what the behavior is)
a = func:
	private x = 3
	ret func:
		printvar x
	end
end
echo 'Variant 1'
(a())()

echo 'Variant 2'
class A
	private x = 3
	y = func:
		printvar x
		ret class
			printvar x
		end
	end
end

A.y()
