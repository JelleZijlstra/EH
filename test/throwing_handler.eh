#!/usr/bin/ehi
# What if something throws in the uncaught exception handler
class Thrower {
  public toString = func: -> (throw 42)
}
throw Thrower.new()
