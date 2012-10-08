#!/usr/bin/ehi
class Stack
  private n = 0
  private s

  public initialize = () => (this.s = [])
  
  public push = func: input
    this.s->n = input
    this.n = this.n + 1
  end
  
  public pop = func:
    printvar this.s
    if this.n == 0
      throw "Stack empty"
    end
    this.n = this.n - 1
    ret this.s->(this.n)
  end
  
  public size = () => this.n
end

s = Stack.new ()
printvar s.size ()
s.push 42
printvar s.size ()
printvar s.pop ()

printvar s.pop ()
