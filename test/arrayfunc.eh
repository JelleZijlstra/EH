#!/usr/bin/ehi
include 'arrayfunc_lib.eh'

printvar arrayfunc([1, 2, 3, 4], func: n; ret 2 * n; end)
printvar arrayfunc(["foo", "bar", "baz"], func: str; ret str->0; end)
# Didn't even know that ehi actually accepts this
printvar arrayfunc([true, 1], (x => x.isA Bool))
# And even this
printvar arrayfunc([true, 1], (x => x.isA Bool))
