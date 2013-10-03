#!/usr/bin/ehi
class Foo
	private bar

	# this, remarkably, works
	public setBar(this.bar) = echo(this.bar)

	public getBar() = this.bar
end

f = Foo()
f.setBar 42
echo(f.getBar())
