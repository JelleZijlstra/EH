<?php
/*
 * Sanitizer.php
 *
 * Static methods for data processing.
 */
abstract class Sanitizer {
	public static /* string */ function trimDoi(/* string */ $in) {
		return trim(preg_replace(
			"/[\.;\(]$|^:|^doi:|^http:\/\/dx\.doi\.org\//", 
			"", trim($in)
		));
	}
	public static /* bool */ function testRegex(/* string */ $in) {
	// tests whether a regex pattern is valid
		ob_start();
		$t = @preg_match($in, 'test');
		ob_end_clean();
		// if regex was invalid, preg_match returned FALSE
		return $t !== false;
	}
	public static /* void */ function printVar(/* mixed */ $in) {
		// print a variable in human-readable form.
		echo self::varToString($in);
	}
		
	public static /* string */ function varToString(/* mixed */ $in) {
		if($in === true) {
			return '(true)';
		} elseif($in === false) {
			return '(false)';
		} elseif($in === NULL) {
			return '(null)';
		} elseif($in === '') {
			return '(empty string)';
		} elseif($in === array()) {
			return '(empty array)';
		} elseif($in instanceof Closure) {
			return '(closure)';
		} elseif(is_object($in) and method_exists($in, '__toString')) {
			return (string) $in;
		} else {
			return print_r($in, true);
		}
	}
}
