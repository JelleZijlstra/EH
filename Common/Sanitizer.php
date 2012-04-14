<?php
/*
 * Sanitizer.php
 *
 * Static methods for data processing.
 */
abstract class Sanitizer {
	public static function trimDoi($in) {
		return trim(preg_replace(
			"/([\.;\(]$|^[:]|^doi:\s*)|^http:\/\/dx\.doi\.org\//", 
			"", trim($in)
		));
	}
	public static function testRegex($in) {
	// tests whether a regex pattern is valid
		ob_start();
		$t = @preg_match($in, 'test');
		ob_end_clean();
		// if regex was invalid, preg_match returned FALSE
		return $t !== false;
	}
}
