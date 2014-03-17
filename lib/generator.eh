# Generator class

class MappingGenerator
	private f
	private iteree

	this.inherit Iterable

	# the class is its own iterator
	public getIterator() = this

	public initialize = func: input, f
		this.iteree = input.getIterator()
		this.f = f
	end

	public hasNext() = this.iteree.hasNext()

	public next() = this.f(this.iteree.next())
end
