#!/usr/bin/ehi
# Probably a useful library routine
Array.fill = func: n, f {
  out = []
  for i in n {
    out->i = f i
  }
  out
}

printvar Array.fill 5, func: input -> input
printvar Array.fill 10, func: input -> input * input
