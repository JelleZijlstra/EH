#!/usr/bin/ehi
# Complex numbers
class Complex
	private a_
	private b_
	
	initialize = func: a, b
		this.a_ = a
		this.b_ = b
	end
	
	public a = () => this.a_
	
	public b = () => this.b_
	
	public abs = () => (this.a_ * this.a_ + this.b_ * this.b_).sqrt()
	
	public operator+ = func: rhs
		Complex.new (this.a() + rhs.a(), this.b() + rhs.b())
	end

	public operator- = func: rhs
		Complex.new (this.a() - rhs.a(), this.b() - rhs.b())
	end
	
	public operator* = func: rhs
		private a = this.a() * rhs.a() - this.b() * rhs.b()
		private b = this.a() * rhs.b() + this.b() * rhs.a()
		Complex.new(a, b)
	end
	
	public toString = func:
		'' + this.a_ + ' + ' + this.b_ + 'i'
	end
end

x = Complex.new(0, 1)
echo(x.toString())
printvar x
echo((x + x).toString())
echo((x.operator* x).toString())
