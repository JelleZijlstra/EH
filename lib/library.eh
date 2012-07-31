#!/usr/bin/ehi
#
# Standard library for the EH language, written in EH
#

# Array functions
Array.map = func: f {
	out = []
	for self as key => value {
		out->key = f value
	}
	ret out
}

Array.reduce = func: f, base {
	out = base
	for self as value {
		out = f out, value
	}
	ret out
}

Array.each = func: f {
	for self as key => value {
		f key, value
	}
}

Array.filter = func: f {
	out = []
	for self as key => value {
		if (f key, value) {
			out->key = value
		}
	}
	ret out
}

# Stack class
class Stack
  private n = 0

  public initialize = func: -> []
  
  public push = func: in
    self->n = in
    this.n = this.n + 1
  end
  
  public pop = func:
    printvar self
    if this.n == 0
      throw "Stack empty"
    end
    this.n = this.n - 1
    ret self->(this.n)
  end
  
  public size = func: -> this.n
end

# List class
class List
  initialize = func: head, tail
    if !(tail.isA List)
      throw ArgumentError.new "List tail must be a List", "List.initialize"
    else
      (head, tail)
    end
  end
  
  # This is hackish
  empty = func:
    real_initialize = List.initialize
    List.initialize = func: -> null
    out = List.new()
    List.initialize = real_initialize
    out
  end
  
  head = func: -> self->0
  tail = func: -> self->1
  
  map = func: f
    if self == null
      empty()
    else
      Cons (f self->0), (self->1).map f
    end
  end
  
  reduce = func: base, f
    if self == null
      base
    else
      f self->0, (self->1.reduce base, f)      
    end
  end
  
  length = func: -> (reduce (0, func: k, rest -> rest + 1))
  
  toString = func:
    if self == null
      "[]"
    else
      ((self->0).toString()) + "::" + ((self->1).toString())
    end
  end
end

Nil = List.empty()
Cons = List.new
