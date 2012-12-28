#!/usr/bin/ehi

# try breaking catch
try
	throw()
catch if 42
	echo "no"
end

try
	throw()
catch if {}
	echo "no"
end
