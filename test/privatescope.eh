#!/usr/bin/ehi
# Test private scope (mostly because I'm curious what the behavior is)
a() = do
	private x = 3
	() => do
		printvar x
	end
end
echo 'Variant 1'
(a())()

echo 'Variant 2'
class A
	private static x = 3
	static y() = do
		printvar x
		class
			printvar x
		end
	end
end

A.y()
