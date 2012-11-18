#!/usr/bin/ehi
class Foo
	private bar
	
	# this, remarkably, works
	public setBar = this.bar => echo bar
	
	public getBar = () => this.bar
end

f = Foo.new()
f.setBar 42
echo(f.getBar())
