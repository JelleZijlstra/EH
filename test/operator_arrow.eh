#!/usr/bin/ehi
include '../lib/tuple.eh'

# Test that we reject invalid index values
trier = func: f
	try
		f()
	catch
		echo exception
	end
end
trier func:
	[]->1.0 = 3
end
trier () => (printvar []->(1..3))
trier func:
	{}->1 = 3
end
trier () => (printvar {}->4)
trier func:
	(1, 2)->'test' = 42
end
trier () => ((1, 2)->'test')
