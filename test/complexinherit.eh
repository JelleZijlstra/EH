#!/usr/bin/ehi
class A {
	private a = 1
	class AA {
		private ap = 10
		public aa: {
			echo a
		}
	}
}
class B {
	private b = 2
	class BB {
		private bp = 20
		public bb: {
			echo b
		}
	}
}
class C {
	private c = 3
	class CC {
		private cp = 30
		this.inherit(A.AA)
		public cc: {
			# If everything is working right, there is no a
			try {
				echo a
			} catch {
				echo()
			}
			echo c
		}
	}
}

class D {
	private d = 4
	class DD {
		private dp = 40
		this.inherit(B.BB)
		this.inherit(C.CC)
		public dd: {
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
		}
	}
}
o = D.new().DD
o.dd()
