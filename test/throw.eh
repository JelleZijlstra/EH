#!/usr/bin/ehi
# Exceptions
funky = func: in {
  if (in % 3) == 0 {
    throw "I don't like numbers that are divisible by three"
  } else {
    ret in
  }
}

trying = func: in {
  try {
    ret funky in
  } catch {
    echo 'Caught it'
    printvar exception
    ret 42
  } finally {
    echo 'I am getting called'
  }
}

printvar trying 2
printvar trying 3
