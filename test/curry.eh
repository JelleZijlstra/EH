#!/usr/bin/ehi
# Currying is possible, though not as elegantly as in OCaml
Array.reduce = func: f {
  arr = this
  ret func: start {
    out = start
    for arr as val {
      out = f out, val
      echo out
    }
    ret out
  }
}
printvar ([1, 2].reduce (func: v, x -> v + 1)) 0
