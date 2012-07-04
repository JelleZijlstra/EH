<?php
/*
 * Autocompleter.php
 *
 * Perform autocompletion of search terms.
 */
interface Autocompleter {
	/*
	 * Constructor takes in an array of input words.
	 */
	public function __construct(array $data);
	
	/*
	 * Takes in a word to autocomplete.
	 * Returns an autocompletion, or FALSE if none is found.
	 */
	public function lookup(/* string */ $word);
}
