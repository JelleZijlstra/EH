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
}
