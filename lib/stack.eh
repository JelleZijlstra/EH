# Stack class
class Stack
	private n = 0
	private arr

	public initialize = () => (this.arr = [])

	public push = func: input
		this.arr->n = input
		this.n++
		null
	end

	public pop = func:
		if this.n == 0
			throw "Stack empty"
		end
		this.n--
		this.arr->(this.n)
	end

	public size = () => this.n
end
