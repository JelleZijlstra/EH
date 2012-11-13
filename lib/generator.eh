# Generator class

class Generator
	private function
	private iteree

	this.inherit Iterable

	# the class is its own iterator
	public getIterator = () => this

	public initialize = func: input, f
		this.iteree = input.getIterator()
		this.function = f
	end

	public hasNext = () => this.iteree.hasNext()

	public next = () => this.function this.iteree.next()
end
