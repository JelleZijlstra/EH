#!/usr/bin/ehi

include '../lib/library.eh'

class Matrix
	private fa
	private size

	public static make_two = func: size, file
		private fd = File.new file
		private m1 = Matrix.new(size, fd: fd)
		private m2 = Matrix.new(size, fd: fd)
		(m1, m2)
	end

	public initialize = func: size, fd: false, f: false
		this.size = size
		if fd != false
			this.fa = FixedArray.fill(size, (_ => FixedArray.fill(size, (_ => fd.gets().toInteger()))))
		elsif f != false
			this.fa = FixedArray.fill(size, (i => FixedArray.fill(size, (j => f(i, j)))))
		else
			this.fa = FixedArray.fill(size, (_ => FixedArray.new size))
		end
	end

	public toString() = this.fa.toString()

	public get(i, j) = (this.fa->i)->j
	public safe_get(i, j) = if i < this.size && j < this.size
		this.get(i, j)
	else
		0
	end
	public set(i, j, v) = ((this.fa->i)->j = v)
	public get_size() = this.size
	public is_odd() = (this.size % 2) == 1
	public half_size() = (this.size + 1) / 2

	private static fold2 = func: f, m1, m2
		private size = m1.get_size()
		private out = Matrix.new(size, fd: false)
		for i in size
			for j in size
				out.set(i, j, f(m1.get(i, j), m2.get(i, j)))
			end
		end
		out
	end

	private static fold_is2 = func: f, m1, m2
		private size = m1.get_size()
		private out = Matrix.new(size, fd: false)
		for i in size
			for j in size
				out.set(i, j, f(i, j))
			end
		end
		out
	end

	public operator+ r = fold2((a, b => a + b), this, r)
	public operator- r = fold2((a, b => a - b), this, r)

	private static multiply(l, r) = fold_is2((i, j => l.get_size().reduce(0, (k, accum => accum + l.get(i, k) * r.get(k, j)))), l, r)
	public operator* r = multiply(this, r)

	public partition = func:
		private hs = this.half_size()
		private is_odd = this.is_odd()
		private that = this

		private lt = Matrix.new(hs, f: (that.get))
		private rt = Matrix.new(hs, f: (i, j => that.safe_get(i, j + hs)))
		private lb = Matrix.new(hs, f: (i, j => that.safe_get(i + hs, j)))
		private rb = Matrix.new(hs, f: (i, j => that.safe_get(i + hs, j + hs)))
		(is_odd, lt, rt, lb, rb)
	end

	public static combine(is_odd, lt, rt, lb, rb) = do
		private half_size = lt.get_size()
		private size = if is_odd; half_size * 2 - 1; else half_size * 2; end
		Matrix.new(size, f: (i, j => match (i / half_size, j / half_size)
			case 0, 0; lt.get(i, j)
			case 0, 1; rt.get(i, j - half_size)
			case 1, 0; lb.get(i - half_size, j)
			case 1, 1; rb.get(i - half_size, j - half_size)
		end))
	end

	private static strassen_mult(a, b) = if a.get_size() < 4
		a * b
	else
		private is_odd, a00, a01, a10, a11 = a.partition()
		private _, b00, b01, b10, b11 = b.partition()
		private m1 = (a00 + a11) ** (b00 + b11)
		private m2 = (a10 + a11) ** b00
		private m3 = a00 ** (b01 - b11)
		private m4 = a11 ** (b10 - b00)
		private m5 = (a00 + a01) ** b11
		private m6 = (a10 - a00) ** (b00 + b01)
		private m7 = (a01 - a11) ** (b10 + b11)
		private c00 = m1 + m4 - m5 + m7
		private c01 = m3 + m5
		private c10 = m2 + m4
		private c11 = m1 - m2 + m3 + m6
		combine(is_odd, c00, c01, c10, c11)
	end

	public operator** r = strassen_mult(this, r)

	public operator== r = if this.get_size() == r.get_size()
		private size = this.get_size()
		for i in size
			for j in size
				if this.get(i, j) != r.get(i, j)
					ret false
				end
			end
		end
		true
	else
		false
	end

	public print_diagonal() = for i in this.size
		echo(this.get(i, i))
	end
end

private m1, m2 = Matrix.make_two(argv->2.toInteger(), argv->3)
match argv->1
	case "0"; (m1 ** m2).print_diagonal()
	case "1"; (m1 * m2).print_diagonal()
	case "2"
		echo(m1 * m2)
		echo(m1 ** m2)
		assert(m1 * m2 == m1 ** m2)
end
