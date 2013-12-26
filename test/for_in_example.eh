#!/usr/bin/ehi

# Custom object to demonstrate EH iteration protocol.
# Behavior is equivalent to that of Integer.
class ExampleIterable
	private n

	public initialize n = (this.n = n)

	public getIterator() = Iterator(this.n)

	class Iterator
		private n
		private i = 0

		public initialize n = do
			this.n = n
			this.i = 0
		end

		public hasNext() = (this.i < this.n)

		public next() = if this.hasNext()
			private out = this.i
			this.i += 1
			out
		else
			throw(EmptyIterator())
		end
	end
end

private ei = ExampleIterable 3

# Use a for-in loop to iterate over an ExampleIterable
for i in ei
	echo i
end

# This is equivalent to the following:
private it_ = ei.getIterator()
while true
	private hasNext_ = it_.hasNext()
	if !(hasNext_.isA Bool)
		throw(TypeError("hasNext does not return a bool", hasNext_.typeId()))
	end
	if hasNext_ == false
		break
	end
	i = it_.next()
	echo i
end

# for-in works on many builtin types, including Tuple
for i in (0, 1, 2)
	echo i
end

# and String
for i in "foo"
	echo i
end
