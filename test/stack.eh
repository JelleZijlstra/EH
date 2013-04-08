#!/usr/bin/ehi
class Stack
	private n = 0
	private s

	public initialize() = (this.s = [])

	public push = func: input
		if this.s.length() <= n
			this.s.push input
		else
			this.s->n = input
		end
		this.n = this.n + 1
	end

	public pop = func:
		printvar(this.s)
		if this.n == 0
			throw "Stack empty"
		end
		this.n = this.n - 1
		this.s->(this.n)
	end

	public size() = this.n
end

s = Stack.new ()
printvar(s.size ())
s.push 42
printvar(s.size ())
printvar(s.pop ())

printvar(s.pop ())
