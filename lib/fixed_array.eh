#!/usr/bin/ehi

FixedArray.inherit class
	class Iterator
		private fa
		private current
		private size

		public initialize = func: fa
			this.fa = fa
			this.current = 0
			this.size = fa.size()
		end

		public hasNext = () => this.current < this.size

		public next = func:
			if this.current < this.size
				private out = this.fa->(this.current)
				this.current++
				out
			else
				throw EmptyIterator.new()
			end
		end
	end

	public getIterator = () => this.Iterator.new this

end

FixedArray.inherit Iterable

FixedArray.toString = func:
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

FixedArray.with = func: input
	private const sizes = input.length()
	private const out = FixedArray.new sizes
	private index = 0
	for i in input
		out->index = i
		index++
	end
	out
end
