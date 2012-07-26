#!/usr/bin/ehi
printvar: 3.isA: Integer
printvar: 4.isA: Bool
class Foo {
}
f = Foo.new: ()
printvar: f.isA: Foo
printvar: (Foo.new: ()).isA: f
