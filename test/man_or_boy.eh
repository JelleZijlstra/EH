#!/usr/bin/ehi

private k = if argc > 1; argv->1.toInteger(); else 10; end

private a k x1 x2 x3 x4 x5 = do
	private b () = do
		k = k - 1
		a k b x1 x2 x3 x4
	end
	if k <= 0
		x4() + x5()
	else
		b()
	end
end

K n () = n

echo(a k (K 1) (K -1) (K -1) (K 1) (K 0))


