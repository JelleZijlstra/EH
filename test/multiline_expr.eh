#!/usr/bin/ehi

if (do x = 3; x == 3; end)
	echo x
else
	echo "impossible"
end

# List class
enum List
	Nil, Cons(head, tail)

	const operator-> n = match this
		case Cons(@hd, @tl); match n
			case 0; hd
			case 1; tl
		end
	end

	class Iterator
		private l

		public initialize l = (this.l = l)
		public hasNext() = this.l != Nil
		public next() = do out, this.l = this.l; out; end
	end

	const getIterator() = Iterator this
end

private l = List.Cons("first", List.Cons("second", List.Nil))

for i in l
	echo i
end

class Nats
	private n

	public getIterator() = this.type().new()
	public initialize() = (this.n = 0)
	public next() = do this.n += 1; this.n - 1; end
	public hasNext() = true
end

for i in Nats()
	echo i
	# some further grammar-twiddling would be required to get rid of the
	# parentheses around "break"
	(i == 4) && (break)
end
