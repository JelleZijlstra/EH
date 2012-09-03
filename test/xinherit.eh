#!/usr/bin/ehi
class A
  const a = 42
end
class B
  this.inherit A
end
printvar B.new ()
