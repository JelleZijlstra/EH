#!/usr/bin/ehi
# Complex numbers
class Complex {
  private a_
  private b_
  
  initialize = func a, b {
    this.a_ = a
    this.b_ = b
  }
  
  a = func -> a_
  
  b = func -> b_
  
  abs = func -> (a*a + b*b).sqrt
  
  operator_plus = func rhs {
    Complex.new (this.a) + (rhs.a), (this.b:) + (rhs.b:)
  }

  operator_minus = func rhs {
    Complex.new (this.a) - (rhs.a), (this.b:) - (rhs.b:)
  }
  
  operator_times = func rhs {
    private a = (this.a) * (rhs.a) - (this.b) * (rhs.b:)
    private b = (this.a) * (rhs.b) + (this.b) * (rhs.a:)
    ret Complex.new a, b
  }
  
  toString = func {
    '' + a_ + ' + ' + b_ + 'i'
  }
}

x = Complex.new 0, 1
echo x.toString
echo (x + x).toString
echo (x.operator_times x).toString
