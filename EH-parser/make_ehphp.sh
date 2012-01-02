#!/bin/bash
# Control the way ehphp is compiled on different OSes
OS=`uname`

case OS in
	'Darwin')
		$1 -dynamiclib $2 -o ehphp.dylib $3
		sudo cp ehphp.dylib /opt/local/lib/php/extensions/no-debug-non-zts-20090626/ehphp.dylib
		;;
	'Linux')
		$1 -shared $2 -o ehphp.so $3
		;;
esac
