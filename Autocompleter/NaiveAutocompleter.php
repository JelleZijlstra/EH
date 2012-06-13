<?php
/*
 * NaiveAutocompleter.php
 *
 * Autocompleter implementation using naive looping over an array.
 */
class NaiveAutocompleter implements Autocompleter {
	private $words;
	
	/*
	 * Public interface
	 */
	public function __construct(array $data) {
		$this->words = $data;
	}
	
	public function lookup(/* string */ $word) {
		$len = strlen($word);
		$matching = array();
		foreach($this->words as $w) {
			if($len > 0 && $len < strlen($w) && substr_compare($w, $word, 0, $len) === 0) {
				$matching[] = substr($w, $len);
			}
		}
		if(count($matching) === 0) {
			return false;
		}
		$out = array_pop($matching);
		foreach($matching as $match) {
			// http://stackoverflow.com/questions/7475437/find-first-character-that-is-different-between-two-strings
			$firstDifference = strspn($out ^ $match, "\0");
			// need this because substr($out, 0) doesn't do what you'd expect
			if($firstDifference === 0) {
				return '';
			}
			$out = substr($out, $firstDifference);
		}
		return $out;
	}
	
	/*
	 * Private functions
	 */
}
