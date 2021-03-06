#!/usr/bin/ehi

FixedArray.inherit class
	class Iterator
		private fa
		private current
		private size

		public initialize fa = do
			this.fa = fa
			this.current = 0
			this.size = fa.size()
		end

		public hasNext() = this.current < this.size

		public next() = if this.current < this.size
			private out = this.fa->(this.current)
			this.current++
			out
		else
			throw(EmptyIterator())
		end
	end

	public getIterator() = Iterator this

end

FixedArray.inherit Iterable

FixedArray##toString() = do
	private out = '['
	const size = this.size()
	for i in size
		out += this->i
		if i < (size - 1)
			out += ','
		end
	end
	out + ']'
end

FixedArray.mapFrom input = do
	private const size = input.length()
	private const out = FixedArray.new size
	private index = 0
	for i in input
		out->index = i
		index++
	end
	out
end

FixedArray.fill(size, f) = do
	out = FixedArray.new size
	for i in size
		out->i = f i
	end
	out
end

FixedArray##reverse() = do
	private size = this.size()
	private out = FixedArray.new size
	for i in size
		out->(size - 1 - i) = this->i
	end
	out
end
