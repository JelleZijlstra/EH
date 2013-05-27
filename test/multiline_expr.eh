#!/usr/bin/ehi

if (x = 3; x == 3)
	echo x
else
	echo "impossible"
end

# List class
enum List
	Nil, Cons(head, tail)

	const operator-> n = match this
		case Cons(@hd, @tl); given n
			case 0; hd
			case 1; tl
			default; throw(ArgumentError.new("Argument must be 0 or 1", "List.operator->", n))
		end
	end

	class Iterator
		private l

		public initialize l = (this.l = l)

		public hasNext() = this.l != Nil

		public next() = (out, this.l = this.l; out)
	end
	const Iterator = Iterator

	const getIterator() = this.Iterator.new this
end

const Nil = List.Nil
const Cons = List.Cons
Object.operator:: rhs = Cons(this, rhs)

private l = List.Cons("first", List.Cons("second", List.Nil))

for i in l
	echo i
end

class Nats
	private n

	public getIterator() = this.new()
	public initialize() = (this.n = 0)
	public next() = (this.n += 1; this.n - 1)
	public hasNext() = true
end

for i in Nats.new()
	echo i
	# some further grammar-twiddling would be required to get rid of the
	# parentheses around "break"
	(i == 4) && (break)
end
