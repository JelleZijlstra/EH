#!/usr/bin/ehi
class CountClass
	private count = 0

	public docount = () => (this.count += 1)

	public setcount = i => (this.count = i)
end

# Test a library class
counter = CountClass.new()
printvar counter
echo(counter.docount())
echo(counter.docount())
