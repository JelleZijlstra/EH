#!/usr/bin/ehi
class A {
	private a = 1
	class AA {
		private ap = 10
		public aa: {
			echo $a
		}
	}
}
class B {
	private b = 2
	class BB {
		private bp = 20
		public bb: {
			echo $b
		}
	}
}
class C {
	private c = 3
	class CC {
		private cp = 30
		inherit $A->AA
		public cc: {
			echo $a
			echo $c
		}
	}
}

class D {
	private d = 4
	class DD {
		private dp = 40
		inherit $B->BB
		inherit $C->CC
		public dd: {
			# Expect 1
			$this->aa:
			# Expect 2
			$this->bb:
			# Expect null, 3
			$this->cc:
			# Expect 4
			echo $d
			
			# Expect 10, 20, 30, 40
			echo $ap
			echo $bp
			echo $cp
			echo $dp
		}
	}
}
o = new $D->DD
$o->dd:
