# Stack class
class Stack
	private n = 0
	private arr

	public initialize = func: -> (this.arr = [])
	
	public push = func: in
		this.arr->n = in
		this.n++
		null
	end
	
	public pop = func:
		if this.n == 0
			throw "Stack empty"
		end
		this.n--
		this.hash->(this.n)
	end
	
	public size = func: -> this.n
end
