#!/usr/bin/ehi
# This throws an error, because intest gets defined twice. This is arguably a
# bug, though PHP's behavior is the same.
func test:
	func intest:
		ret 42
	end
end
test:
test:
