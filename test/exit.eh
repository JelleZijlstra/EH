#!/usr/bin/ehi

try
	exit()
catch if exception.isA TypeError
	echo exception
end

exit 0
echo "This won't get executed"
