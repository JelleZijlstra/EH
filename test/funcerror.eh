#!/usr/bin/ehi
# This now does not throw an error because of lexical scope.
func test:
	func intest:
		ret 42
	end
end
test: ()
test: ()
