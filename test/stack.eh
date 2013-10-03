#!/usr/bin/ehi
class Stack
	private n = 0
	private s

	public initialize() = (this.s = [])

	public push input = do
		if this.s.length() <= this.n
			this.s.push input
		else
			this.s->this.n = input
		end
		this.n = this.n + 1
	end

	public pop() = do
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
