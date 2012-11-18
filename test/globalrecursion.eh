#!/usr/bin/ehi
# Can printvar handle this?
class B; end
class A
	public foo = B
end
B.o = A.new()
printvar B
