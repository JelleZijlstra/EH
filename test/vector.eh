#!/usr/bin/ehi
# Inheriting from inbuilt types
class Vector
  inherit Array
  
  public initialize = func: -> []
  
  private set = operator->=
  
  public operator->= = func: index, value
    if !(index.isA Integer)
      throw "Invalid type for argument 0 to Vector.operator->="
    end
    set index, value
  end
  
  private get = operator->
  
  public operator-> = func: index
    if !(index.isA Integer)
      throw "Invalid type for argument 0 to Vector.operator->"
    end
    get index
  end
end

v = Vector.new ()
printvar v
v->0 = 42
printvar v->0
printvar v.length ()
