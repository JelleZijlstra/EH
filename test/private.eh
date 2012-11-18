#!/usr/bin/ehi
foo = 2
baz = 4
class Bar
	private foo = 3
	public tryIt:
		# 3
		echo foo
		# 3
		echo(this.foo)
		# 4
		echo baz
		# error
		try
			echo(this.baz)
		catch
			echo(exception.toString())
		end
		this.baz = 5
		# 5
		echo(this.baz)
		# 5
		echo baz
	end

	const toString = () => "@Bar " + foo
end
# 2
echo foo
Bar.tryIt ()
