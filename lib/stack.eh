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
