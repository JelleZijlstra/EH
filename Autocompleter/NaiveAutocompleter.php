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
	
	// Returns an autocompletion, or FALSE if none is found
	public function lookup(/* string */ $word) {
		$len = strlen($word);
		$matching = array();
		foreach($this->words as $w) {
			if($len < strlen($w) && substr_compare($w, $word, 0, $len) === 0) {
				$matching[] = substr($w, $len);
			}
		}
		if(count($matching) === 0) {
			return false;
		}
		$out = array_pop($matching);
		foreach($matching as $match) {
			// http://stackoverflow.com/questions/7475437/find-first-character-that-is-different-between-two-strings
			$out = substr($out, strspn($out ^ $match, "\0"));
			
			// possible optimization: return immediately if string is already
			// length 0
			if($out === '') {
				break;
			}
		}
		return '';
	}
	
	/*
	 * Private functions
	 */
}
