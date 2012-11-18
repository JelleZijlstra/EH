#!/usr/bin/ehi
# Illustrate the use of iterators, as well as EH's flexibility

class Iterable
	public each = func: f
		i = this.getIterator()
		while i.hasNext()
			f(i.next())
		end
	end
end

String.inherit Iterable
"foo".each printvar

Range.inherit Iterable
1..3.each printvar

Hash.inherit Iterable
{foo: 'bar'}.each printvar

Array.inherit Iterable
[1, 2, 3].each printvar

['foo' => 'bar', 'baz' => 'mah'].each func: k, v
	echo('Key is ' + k)
	echo('Value is ' + v)
end

Tuple.inherit Iterable
(1, 2, 3).each printvar

Integer.inherit Iterable
5.each printvar
