#!/bin/bash
# Control the way ehphp is compiled on different OSes
OS=`uname`

case $OS in
	'Darwin')
		$1 -dynamiclib $2 -o ehphp.dylib $3
		if [ -e /opt/local/lib/php55/extensions/no-debug-non-zts-20121212 ]; then
			sudo cp ehphp.dylib /opt/local/lib/php55/extensions/no-debug-non-zts-20121212/ehphp.dylib
			#sudo cp ehphp.dylib /opt/local/lib/php54/extensions/debug-non-zts-20100525/ehphp.dylib
		fi
		;;
	'Linux')
		$1 -shared -o ehphp.so $3
		sudo cp ehphp.so /usr/lib/php/modules/ehphp.so
		;;
esac
