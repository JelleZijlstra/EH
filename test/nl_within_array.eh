#!/usr/bin/ehi

printvar [
	'foo' => 'bar',
	3,
	5,
	'hello'
]

printvar {
	foo: 'bar',
	baz: 42,
	quux: []
}
