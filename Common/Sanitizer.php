<?php
/*
 * Sanitizer.php
 *
 * Static methods for data processing.
 */
abstract class Sanitizer {
	public static /* string */ function trimDoi(/* string */ $in) {
		return trim(preg_replace(
			"/([\.;\(]$|^[:]|^doi:\s*)|^http:\/\/dx\.doi\.org\//", 
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
		if($in === true) {
			echo '(true)';
		} elseif($in === false) {
			echo '(false)';
		} elseif($in === NULL) {
			echo '(null)';
		} elseif($in === '') {
			echo '(empty string)';
		} elseif($in === array()) {
			echo '(empty array)';
		} elseif($in instanceof Closure) {
			echo '(closure)';
		} else {
			print_r($in);
		}
	}
}
