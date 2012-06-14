<?php
/*
 * OneStepAutocompleter.php
 *
 * Autocompleter implementation using naive looping over an array.
 */
class OneStepAutocompleter implements Autocompleter {
	private $words;
	
	/*
	 * Public interface
	 */
	public function __construct(array $data) {
		$this->words = $data;
	}
	
	public function lookup(/* string */ $word) {
		$len = strlen($word);
		$out = false;
		foreach($this->words as $w) {
			// the $word[0] === $w[0] check appears to improve performance by ~25%
			if($len > 0 && ($word[0] === $w[0]) && $len < strlen($w) && substr_compare($w, $word, 0, $len) === 0) {
				$newWord = substr($w, $len);
			} elseif($len === 0) {
				$newWord = $w;
			} else {
				continue;
			}
			if($out === false) {
				$out = $newWord;
			} else {
				// http://stackoverflow.com/questions/7475437/find-first-character-that-is-different-between-two-strings
				$firstDifference = strspn($out ^ $newWord, "\0");
				// need this because substr($out, 0) doesn't do what you'd expect
				if($firstDifference === 0) {
					return '';
				}
				$out = substr($out, 0, $firstDifference);
			}
		}
		return $out;
	}
	
	/*
	 * Private functions
	 */
}
