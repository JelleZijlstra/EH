#!/usr/bin/ehi
class A
	private static a = 1
	class AA
		private ap = 10
		public aa() = echo a
	end
end
class B
	private static b = 2
	class BB
		private bp = 20
		public bb() = echo b
	end
end
class C
	private static c = 3
	class CC
		private cp = 30
		this.inherit(A.AA)
		public cc() = do
			# If everything is working right, there is no a
			try
				echo a
			catch
				echo()
			end
			echo c
		end
	end
end

class D
	private static d = 4
	class DD
		private dp = 40
		this.inherit(B.BB)
		this.inherit(C.CC)
		public dd() = do
			# Expect 1
			this.aa()
			# Expect 2
			this.bb()
			# Expect null, 3
			this.cc()
			# Expect 4
			echo d

			# Expect 10, 20, 30, 40
			echo(this.ap)
			echo(this.bp)
			echo(this.cp)
			echo(this.dp)
		end
	end
end
o = D.DD()
o.dd()
