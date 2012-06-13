<?php
/*
 * FixedArrayAutocompleter.php
 *
 * Autocompleter implementation using SplFixedArray.
 * Assumes words contain only ASCII characters 32–126 inclusive.
 */
class FixedArrayAutocompleter implements Autocompleter {
	private $tree;
	
	const NUM_CHARACTERS = 96;
	const CHARACTER_OFFSET = 31;
	
	/*
	 * Public interface
	 */
	public function __construct(array $data) {
		$this->tree = new SplFixedArray(self::NUM_CHARACTERS);
		foreach($data as $word) {
			$this->fill($this->tree, $word);
		}
	}
	
	// Returns an autocompletion, or FALSE if none is found
	public function lookup(/* string */ $word) {
		$tree = $this->tree;
		for($i = 0, $len = strlen($word); $i < $len; $i++) {
			$c = self::characterToOffset($word[$i]);
			if(!isset($tree[$c])) {
				return false;
			}
			$tree = $tree[$c];
		}
		// now perform the actual lookup
		$out = '';
		while(true) {
			$currentKey = false;
			$currentValue = false;
			foreach($tree as $key => $value) {
				if($value !== NULL) {
					if($currentKey !== false) {
						break 2;
					} else {
						$currentKey = $key;
						$currentValue = $value;
					}
				}
			}
			$out .= self::offsetToCharacter($currentKey);
			if($currentValue === true) {
				break;
			}
			$tree = $currentValue;
		}
		return $out;
	}
	
	/*
	 * Private functions
	 */
	private function fill(&$tree, $word) {
		if(strlen($word) === 0) {
			$tree[0] = true;
			return;
		}
		$firstLetter = self::characterToOffset($word[0]);
		$rest = substr($word, 1);
		if(!isset($tree[$firstLetter])) {
			$tree[$firstLetter] = new SplFixedArray(self::NUM_CHARACTERS);
		}
		$this->fill($tree[$firstLetter], $rest);
	}
	
	private static function characterToOffset(/* char */ $c) {
		return ord($c) - self::CHARACTER_OFFSET;
	}
	private static function offsetToCharacter(/* int */ $o) {
		return chr($o + self::CHARACTER_OFFSET);
	}
	
	private static function arrayHasMoreThanOneElement(SplFixedArray $in) {
		$one = false;
		foreach($in as $value) {
			if($value !== NULL) {
				if($one) {
					return false;
				} else {
					$one = true;
				}
			}
		}
		return $one;
	}
}
