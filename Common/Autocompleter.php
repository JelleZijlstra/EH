<?php
/*
 * Autocompleter.php
 *
 * Perform autocompletion of search terms.
 */
class Autocompleter {
	private $tree = array();
	
	/*
	 * Public interface
	 */
	public function __construct(array $data) {
		foreach($data as $word) {
			$this->fill($this->tree, $word);
		}
	}
	
	// Returns an autocompletion, or FALSE if none is found
	public function lookup(/* string */ $word) {
		$tree = $this->tree;
		for($i = 0, $len = strlen($word); $i < $len; $i++) {
			$c = $word[$i];
			if(!isset($tree[$c])) {
				return false;
			}
			$tree = $tree[$c];
		}
		// now perform the actual lookup
		$out = '';
		while(true) {
			if(count($tree) === 1) {
				// most convenient way to get the key and value out
				foreach($tree as $key => $value) {
					if($key === 'END') {
						break 2;
					} else {
						$out .= $key;
						$tree = $value;
					}
				}
			} else {
				break;
			}
		}
		return $out;
	}
	
	/*
	 * Private functions
	 */
	private function fill(&$tree, $word) {
		if(strlen($word) === 0) {
			$tree['END'] = true;
			return;
		}
		$firstLetter = $word[0];
		$rest = substr($word, 1);
		if(!isset($tree[$firstLetter])) {
			$tree[$firstLetter] = array();
		}
		$this->fill($tree[$firstLetter], $rest);
	}
}
