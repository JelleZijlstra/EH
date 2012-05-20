<?php
/*
 * NameParser. A class that can parse file names into machine-readable units.
 */

class NameParser {
	static private $geographicTerms;
	static private $geographicModifiers;
	static private $geographicWords;
	static private $periodTerms;
	static private $periodModifiers;
	static private $periodWords;
	
	/*
	 * Whether an error occurred, and a description.
	 */
	private $errorDescription = array();
	
	public function errorOccurred() { 
		return $this->errorDescription !== array(); 
	}
	
	public function getErrors() { return $this->errorDescription; }
	
	public function printErrors() {
		echo count($this->getErrors()) . ' errors while parsing name: ' 
			. $this->rawName . PHP_EOL;
		foreach($this->getErrors() as $error) {
			echo '- ' . $error . PHP_EOL;
		}
	}
	
	private function addError($description) {
		$description = trim($description);
		$this->errorDescription[] = $description;
	}
	
	/*
	 * Print as parsed.
	 */
	public function printParsed() {
		echo 'Parsing name: ' . $this->rawName . PHP_EOL;
		$printIfNotEmpty = function($field) {
			if($this->$field !== '') {
				echo ucfirst($field) . ': ' 
					. Sanitizer::varToString($this->$field) . PHP_EOL;
			}
		};
		$printIfNotEmpty('extension');
		$printIfNotEmpty('modifier');
		if($this->authorship !== array(false, '')) {
			echo 'Authorship:' . PHP_EOL;
			if($this->authorship[0] !== false) {
				echo "\tAuthors: " . implode('; ', $this->authorship[0]) 
					. PHP_EOL;
			}
			echo "\tYear: " . $this->authorship[1] . PHP_EOL;
		}
		$printIfNotEmpty('baseName');
		echo PHP_EOL;
	}

	/*
	 * The raw file name parsed.
	 */
	private $rawName = '';
	public function rawName() { return $this->rawName; }

	/*
	 * The file extension (e.g., "pdf").
	 */
	private $extension = '';
	public function extension() { return $this->extension; }
	
	/*
	 * Modifier (e.g., "part 2").
	 */
	private $modifier = '';
	public function modifier() { return $this->modifier; }
	
	/*
	 * Authorship, when included in the title. This will return an array
	 * of two elements. The first element will be false if only a year is given, 
	 * a string with the name of an author if one author is given, an array with
	 * multiple elements if multiple authors are given, and an array with a
	 * single element if "et al." is given. The second element is the year.
	 */
	private $authorship = array(false, '');
	public function authorship() { return $this->authorship; }
	
	/*
	 * baseName is an array with elements representing parts of the title. The
	 * keys may be "nov" or "normal", representing <nov-phrase> and 
	 * <normal-name>.
	 */
	private $baseName = array();
	public function baseName() { return $this->baseName; }
	
	public function __construct(/* string */ $name) {
		self::buildLists();
		$this->rawName = $name;
		$name = $this->splitExtension($name);
		$name = $this->splitModifiers($name);
		/*
		 * Now, a name may be:
		 * (1) <nov-phrase> 
		 *		[e.g., "Agathaeromys nov" or "Dilambdogale, Qatranilestes nov"]
		 * (2) <normal-name>
		 *		[e.g., "Afrosoricida Egypt Eo-Oligocene.pdf"]
		 * (3) <nov-phrase>, <normal-name>
		 *
		 * TODO: other possibilities. "for-phrases" (e.g., 
		 * "Churcheria for Anonymus.pdf"), stuff with parasites.
		 */
		if(substr($name, -3, 3) === 'nov') {
			// possibility (1)
			$this->parseNovPhrase($name);
		} elseif(strpos($name, 'nov, ') !== false) {
			$parts = preg_split('/(?<=nov), /u', $name);
			$this->parseNovPhrase($parts[0]);
			$this->parseNormalName($parts[1]);
		} else {
			$this->parseNormalName($name);
		}
	}
	
	/*
	 * Parsing functions.
	 */
	private function splitExtension($name) {
		if(preg_match('/^(.*)\.([a-z]+)$/u', $name, $matches)) {
			$this->extension = $matches[2];
			return $matches[1];
		} else {
			return $name;
		}
	}
	
	/*
	 * A name may end in "(<author-modifier>)? (<free-modifier>)?". Any other
	 * pattern of parenthesized expressions is an error.
	 */	
	private function splitModifiers($name) {
		$firstSet = $this->splitParentheses($name);
		if(!is_array($firstSet)) {
			return $name;
		}
		$name = $firstSet[0];
		if($this->isAuthorModifier($firstSet[1])) {
			// one modifier
			$this->parseAuthorModifier($firstSet[1]);
		} else {
			// last modifier is free-form
			$this->modifier = $firstSet[1];
			$secondSet = $this->splitParentheses($name);
			if(!is_array($secondSet)) {
				return $name;
			}
			$name = $secondSet[0];
			// possible author modifier
			if($this->isAuthorModifier($secondSet[1])) {
				$this->parseAuthorModifier($secondSet[1]);
			} else {
				$this->addError('Too many modifiers');
				return $name;
			}
		}
		// any other modifiers now would be an error, but check
		$thirdSet = $this->splitParentheses($name);
		if(is_array($thirdSet)) {
			$this->addError('Too many modifiers');
			// attempt error recovery, but if there are even more modifiers even
			// this won't work
			return $thirdSet[0];
		} else {
			return $name;
		}
	}
	
	private function splitParentheses($in) {
		if(preg_match('/^(.*) \(([^()]+)\)$/u', $in, $matches)) {
			return array($matches[1], $matches[2]);
		} else {
			return $in;
		}
	}
	
	private function isAuthorModifier($in) {
		return preg_match('/\d{4}$/', $in);
	}
	
	private function parseAuthorModifier($in) {
		// <author-modifier> is <authors>? year
	
		// this should always match, because we assume that input passed the
		// isAuthorModifier function
		preg_match('/^(.*)(\d{4})$/u', $in, $matches);
		// year
		$this->authorship[1] = (int) $matches[2];
		$authors = trim($matches[1]);
		if($authors === '') {
			// nothing to do
			return;
		}
		// <authors> may be "A", "A & B" or "A et al."
		if(substr($authors, -6, 6) === 'et al.') {
			$this->authorship[0] = array(substr($authors, 0, -7));
		} elseif(strpos($authors, ' & ') !== false) {
			$this->authorship[0] = explode(' & ', $authors);
		} else {
			$this->authorship[0] = $authors;
		}
	}
	
	/*
	 * Nov phrases
	 *
	 * Nov phrases have the forms:
	 * (1) <group> <number>nov
	 *		[Oryzomyini 10nov]
	 * (2) <scientific name> nov
	 *		[Agathaeromys nov]
	 * (3) <scientific name>, <abbreviated scientific name> nov
	 *		[Murina beelzebub, cinerea, walstoni nov]
	 *
	 * (1) is parsed into 'nov' => array(array(10, 'Oryzomyini'))
	 * (2) is parsed into 'nov' => array('Agathaeromys')
	 * (3) becomes 'nov' => array(
	 		'Murina beelzebub', 'Murina cinerea', 'Murina walstoni'
	 * )
	 */
	private function parseNovPhrase($in) {
		$nov = array();
		if(preg_match('/^(\w+) (\d+)nov$/u', $in, $matches)) {
			$nov[] = array((int) $matches[2], $matches[1]);
		} else {
			if(!preg_match('/^(.*) nov$/u', $in, $matches)) {
				$this->addError('Invalid nov phrase');
				return;
			}
			$nov = self::parseNames($matches[1]);
		}
		$this->baseName['nov'] = $nov;
	}
	
	/*
	 * Normal names
	 *
	 * Grammar:
	 *  <name-list>? <geography>? <time>? ((-<topic>)?
	 * Where:
	 *  <name-list> is as in a nov phrase
	 *  <geography> is of the form:
	 *		<geographic-modifier>? <geographic-area> <geographic-term>?
	 *  <time> is of the form
	 * 		<period-modifier>? <period>
	 *  (or a more complicated range)
	 *  <topic> is an arbitrary string
	 *
	 * This produces something of the form 'normal' => array(
	 *		'names' => (array as for nov-phrase)
	 *		'geography' => (array of arrays of two items, representing the 
	 *						general and specific geography)
	 *		'time' => (array of items representing either a single time unit as
	 *						a string or an array of two; time unit is 
	 *						represented as array of modifier + major unit)
	 *		'topic' => (array of topics)
	 */
	private function parseNormalName($in) {
		$out = array();
		// first, consume names until we find a geographic or period term
		$names = '';
		while(true) {
			if($in === '') {
				// we're done
				break;
			}
			if($in[0] === '-') {
				$out += $this->parseNormalAtTopic($in);
				break;
			}
			if(self::findWordFromArray($in, self::$periodWords) !== false) {
				$out += $this->parseNormalAtTime($in);
				break;
			}
			if(self::findWordFromArray($in, self::$geographicWords) !== false) {
				$out += $this->parseNormalAtGeography($in);
				break;
			}
			$tmp = self::getFirstWord($in);
			$names .= ' ' . $tmp[0];
			$in = trim($tmp[1]);
		}
		if($names !== '') {
			$out['names'] = self::parseNames(trim($names));
		}
		$this->baseName['normal'] = $out;
	}
	
	private function parseNormalAtTopic($in) {
		// input begins with -
		$topic = explode(', ', substr($in, 1));
		return array('topic' => $topic);
	}
	
	private function parseNormalAtTime($in) {
		// Here we can assume that period terms are always one word
		$out = array();
		$times = array();
		$inRange = false;
		while(1) {
			var_dump($in);
			$split = self::getFirstWord($in);
			$firstWord = $split[0];
			if(in_array($firstWord, self::$periodModifiers)) {
				// next word must be a periodTerm followed by [,-]
				$split = self::getFirstWord(trim($split[1]));
				$secondWord = $split[0];
				if(!in_array($secondWord, self::$periodTerms)) {
					$this->addError('Period modifier not followed by period term');
					break;
				}
				$time = array($firstWord, $secondWord);
			} elseif(in_array($firstWord, self::$periodTerms)) {
				$time = array(NULL, $secondWord);
			} else {
				$this->addError('Invalid word in period');
				break;
			}
			$in = trim($split[1]);
			if($in === '' || ($in[0] === ',' && isset($in[1]) && $in[1] === ' ')) {
				if($inRange) {
					$times[] = array(array_pop($times), $time);
				} else {
					$times[] = $time;
				}
				if($in === '') {
					break;
				}
			} elseif($in[0] === '-') {
				$in = substr($in, 1);			
				if(self::findWordFromArray($in, self::$periodWords)) {
					$inRange = true;
				} else {
					$out += $this->parseNormalAtTopic('-' . $in);
				}
			} else {
				$this->addError('Syntax error in period');
				break;
			}
		}
		if($times !== array()) {
			$out['times'] = $times;
		}
		return $out;
	}
	
	private function parseNormalAtGeography($in) {
		// first find a major term, then minor terms
		$out = array();
		$places = array();
		$currentMajor = '';
		$currentMinor = '';
		while(1) {
			if($in === '') {
				$places[] = array($currentMajor, $currentMinor);
				break;			
			}
			if($currentMinor === '') {
				$findMajor = 
					self::findWordFromArray($in, self::$geographicTerms);
				if($findMajor === false) {
					if($currentMajor === '') {
						$this->addError('Invalid geography');
						break;
					} else {
						// retain $currentMajor and $in
					}
				} else {
					$currentMajor = $findMajor[1];
					$in = $findMajor[0];
				}
			}
			$in = trim($in);
			var_dump($in);
			if($in[0] === ',') {
				$places[] = array($currentMajor, $currentMinor);
				$currentMinor = '';
				$in = trim(substr($in, 1));
			} elseif($in[0] === '-') {
				// decide whether this starts the topic (if there's another - 
				// in the text or the last word is a period, we assume it 
				// doesn't)
				if(strpos($in, '-', 1) === false 
					&& !in_array(self::getLastWord($in), self::$periodTerms)) {
					$places[] = array($currentMajor, $currentMinor);
					$out += $this->parseNormalAtTopic($in);
					break;
				} else {
					$currentMinor .= '-';
					$tmp = self::getFirstWord(substr($in, 1));
					$currentMinor .= $tmp[0];
					$in = $tmp[1];
				}
			} elseif(self::findWordFromArray($in, self::$periodWords) !== false) {
				$places[] = array($currentMajor, $currentMinor);
				$out += $this->parseNormalAtTime($in);
				break;
			} else {
				// then it's a minor term
				if($currentMinor !== '') {
					$currentMinor .= ' ';
				}
				$tmp = self::getFirstWord($in);
				$currentMinor .= $tmp[0];
				$in = $tmp[1];
			}
		}
		$out['geography'] = $places;
		return $out;
	}
	
	/*
	 * Data handling.
	 */
	private static $didBuildLists = false;
	private static function buildLists() {
		if(self::$didBuildLists) {
			return;
		}
		$getData = function($fileName) {
			return array_filter(
				file(BPATH . '/Catalog/data/' . $fileName, 
					FILE_IGNORE_NEW_LINES|FILE_SKIP_EMPTY_LINES),
				function($in) {
					// allow comments with #
					return $in[0] !== '#';
				}
			);
		};
		self::$geographicTerms = $getData('geography.txt');
		self::$geographicModifiers = $getData('geography_modifiers.txt');
		self::$periodTerms = $getData('periods.txt');
		self::$periodModifiers = $getData('period_modifiers.txt');
		
		// overall arrays that are sometimes useful
		self::$geographicWords = 
			array_merge(self::$geographicTerms, self::$geographicModifiers);
		self::$periodWords =
			array_merge(self::$periodTerms, self::$periodModifiers);
			
		self::$didBuildLists = true;
	}
	
	/*
	 * Helper methods.
	 */
	private static function getFirstWord($in) {
		$out = preg_split('/(?=[ \-,])/u', $in, 2);
		// simplify life for callers
		if(!isset($out[1])) {
			$out[1] = '';
		}
		return $out;
	}
	
	private static function getLastWord($in) {
		$out = preg_split('/(?=[ \-])/u', $in);
		return trim(array_pop($out));
	}
	
	/*
	 * Finds whether any of the phrases in array terms occur in haystack.
	 * Returns an array of the haystack without the word plus the word, or
	 * false on failure to find a word.
	 */
	private static function findWordFromArray($haystack, array $terms) {
		foreach($terms as $term) {
			if(strpos($haystack, $term) === 0) {
				$newHay = substr($haystack, strlen($term));
				if($newHay[0] === ',' || $newHay[0] === ' ' || $newHay[0] === '-') {
					return array($newHay, $term);
				}
			}
		}
		return false;
	}
	
	/*
	 * Parse a listing of scientific names.
	 */
	private static function parseNames($in) {
		$out = array();
		$names = explode(', ', $in);
		$lastName = false;
		foreach($names as $name) {
			if(ctype_lower($name[0])) {
				if($lastName === false) {
					$this->addError('Invalid lowercase name');
				} else {
					$name = self::getFirstWord($lastName)[0] . ' ' . $name;
				}
			}
			$out[] = $name;
			$lastName = $name;
		}
		return $out;
	}
}
