#!/usr/bin/ehi
rescue = func: f
	try
		f()
	catch
		echo exception
	end
end
