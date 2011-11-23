<?php
// Implementation of the McCarthy function in PHP
function mccarthy($n) {
	if($n > 100)
		return $n - 10;
	else
		return mccarthy(mccarthy($n+11));
}

echo mccarthy($argv[1]);
?>
