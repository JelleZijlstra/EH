#!/usr/bin/ehi
# Complex numbers
class Complex {
	private a_
	private b_
	
	initialize = func: a, b {
		this.a_ = a
		this.b_ = b
	}
	
	public a = func: -> this.a_
	
	public b = func: -> this.b_
	
	public abs = func: -> (this.a_ * this.a_ + this.b_ * this.b_).sqrt
	
	public operator+ = func: rhs {
		Complex.new (this.a()) + (rhs.a()), (this.b()) + (rhs.b())
	}

	public operator- = func: rhs {
		Complex.new (this.a()) - (rhs.a()), (this.b()) - (rhs.b())
	}
	
	public operator* = func: rhs {
		private a = (this.a()) * (rhs.a()) - (this.b()) * (rhs.b())
		private b = (this.a()) * (rhs.b()) + (this.b()) * (rhs.a())
		Complex.new a, b
	}
	
	public toString = func: {
		'' + this.a_ + ' + ' + this.b_ + 'i'
	}
}

x = Complex.new 0, 1
echo x.toString()
printvar x
echo (x + x).toString()
echo (x.operator* x).toString()
