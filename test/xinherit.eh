#!/usr/bin/ehi
class A
  const a = 42
end
class B
  inherit: A
end
printvar: B.new: ()
