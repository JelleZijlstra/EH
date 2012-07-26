#!/usr/bin/ehi
# The different ways of throwing an exception
try {
  try {
    throw: 42
  } catch {
    printvar: exception
  }
  try
    throw: 43
  catch
    printvar: exception
  end
  try
    throw: 44
  finally
    $echo 'This is important'
  end
} catch {
  $echo 'got it at the top level'
  printvar: exception
}

try {
  throw: 45
} finally {
  $echo 'Not catching it!'
}
