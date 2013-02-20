#!/usr/bin/ehi
# Test decompiling an enum class

private testf = func:
	enum A
		B, C(a, b), D(e)
	end

	enum B
		C, D(a, b)

		private f = () => 42
	end
end

echo(testf.decompile())
