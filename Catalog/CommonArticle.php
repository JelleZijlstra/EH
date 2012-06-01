<?php
/*
 * CommonArticle.
 *
 * Functionality shared by the CSV and SQL implementations of Article.
 */
trait CommonArticle {
	/*
	 * Properties that have a one-to-one correspondence with the database.
	 */
	public /* string */ $name; // name of file (or handle of citation)
	public /* int */ $type; // type of file
	public /* string|int */ $year; // year published
	public /* string */ $title; // title (chapter title for book chapter; book title for full book or thesis)
	public /* Journal / string */ $journal; // journal published in
	public /* string */ $series; // journal series
	public /* string */ $volume; // journal volume
	public /* string */ $issue; // journal issue
	public /* string */ $start_page; // start page
	public /* string */ $end_page; // end page
	public /* string */ $pages; // number of pages in book
	public /* Article / string */ $parent; // enclosing article
	public /* Publisher / string */ $publisher; // publisher
	public /* string */ $misc_data; // miscellaneous data
	public /* Author array / string */ $authors;

	private $pdfcontent; // holds text of first page of PDF
	protected static $Article_commands = array(
		'edittitle' => array('name' => 'edittitle',
			'aka' => array('t'),
			'desc' => 'Edit the title',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'echocite' => array('name' => 'echocite',
			'aka' => array('c', 'cite'),
			'desc' => 'Get a citation for this file',
			'arg' => 'Optionally, citation type to be used',
			'execute' => 'callmethod'),
		'echopdfcontent' => array('name' => 'echopdfcontent',
			'aka' => array('echopdf'),
			'desc' => 'Print the contents of the first page of the PDF file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'openf' => array('name' => 'openf',
			'aka' => array('o', 'open'),
			'desc' => 'Open the file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'openurl' => array('name' => 'openurl',
			'aka' => array('u'),
			'desc' => 'Open the URL associated with the file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'searchgoogletitle' => array('name' => 'searchgoogletitle',
			'aka' => array('g'),
			'desc' => 'Search for the file\'s title in Google',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'removefirstpage' => array('name' => 'removefirstpage',
			'aka' => array('rmfirst'),
			'desc' => 'Remove the first page from the file\'s PDF',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'expanddoi' => array('name' => 'expanddoi',
			'desc' => 'Retrieve data for this DOI',
			'arg' => 'None required',
			'execute' => 'callmethod'),
		'format' => array('name' => 'format',
			'desc' => 'Format the file',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'path' => array('name' => 'path',
			'desc' => 'Return the path to the file',
			'arg' => 'None',
			'execute' => 'callmethod'),
	);

	/*
	 * Some wrappers.
	 */
	abstract protected function publisher();
	abstract protected function location();
	abstract protected function journal();
	abstract protected function doi();
	abstract protected function jstor();
	abstract protected function hdl();
	protected function getIdentifier($paras) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'toarray' => 'name',
			'synonyms' => array(
				0 => 'name',
				'p' => 'print',
			),
			'checklist' => array(
				'name' => 'Name of the identifier to be returned',
				'print' => 'Whether to print the identifier',
			),
			'errorifempty' => array(
				'name',
			),
			'default' => array(
				'print' => false,
			),
			'listoptions' => array(
				'name' => array(
					'url', 'doi', 'pmid', 'jstor', 'pmc', 'hdl', 'isbn',
				),
			),
		))) return false;
		$id = $this->_getIdentifier($paras['name']);
		if(($id !== false) && $paras['print']) {
			Sanitizer::printVar($id);
			echo PHP_EOL;
		}
		return $id;
	}
	abstract protected function _getIdentifier($name);
	
	private $nameParser = NULL;
	public function getNameParser() {
		if($this->nameParser === NULL) {
			$this->nameParser = new NameParser($this->name);
		} elseif($this->nameParser->rawName() !== $this->name) {
			$this->nameParser = new NameParser($this->name);		
		}
		return $this->nameParser;
	}

	/*
	 * Basic operations with files.
	 */
	abstract protected function needSave();
	public function inform(array $paras = array()) {
	// provide information for a file
		// call the parent's basic inform() method
		parent::inform($paras);
		// provide ls data
		if($this->isfile()) {
			$this->shell(array(
				'cmd' => 'ls',
				'arg' => array('-l', $this->path())
			));
		}
	}
	public function path(array $paras = array()) {
	// returns path to file
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'type' => 'Type of path: shell, url, or none',
				'folder' => 'Whether we want the folder only',
				'fullpath' => 'Whether the full path should be returned',
				'print' => 'Whether the result should be printed',
			),
			'default' => array(
				'type' => 'none',
				'folder' => false,
				'fullpath' => true,
				'print' => false,
			),
			'listoptions' => array(
				'type' => array('shell', 'url', 'none'),
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(!$this->isfile()) {
			return false;
		}
		$out = $this->_path($paras);
		if($paras['fullpath']) {
			$out = LIBRARY . $out;
		}
		if(!$paras['folder']) {
			$out .= '/' . $this->name;
		}
		// process output
		switch($paras['type']) {
			case 'shell':
				$out = escapeshellarg($out);
				break;
			case 'url':
				$out = str_replace('%2F', '/', rawurlencode($out));
				break;
			case 'none':
				break;
		}
		if($paras['print']) {
			echo $out . PHP_EOL;
		}
		return $out;
	}
	abstract protected function _path(array $paras);
	public function openf(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'place' => 'Place where the file is located',
			),
			'default' => array(
				'place' => 'catalog',
			),
			'listoptions' => array(
				'place' => array('catalog', 'temp'),
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(!$this->isfile()) {
			echo 'openf: error: not a file, cannot open' . PHP_EOL;
			return false;
		}
		switch($paras['place']) {
			case 'catalog':
				$place = $this->path(array('type' => 'none'));
				break;
			case 'temp':
				$place = TEMPPATH . "/" . $this->name;
				break;
		}
		$this->shell(array(
			'cmd' => 'open',
			'arg' => array($place),
		));
		return true;
	}
	public function remove(array $paras = array()) {
	// remove a file
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array('f' => 'force'),
			'checklist' => array('force' => 'Do not ask for confirmation'),
			'default' => array('force' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($paras['force']) {
			$cmd = true;
		} else {
			$cmd = $this->ynmenu('Are you sure you want to remove file ' . $this->name . '?');
		}
		if($cmd) {
			if($this->isfile()) {
				$this->shell(array('rm', array($this->path())));
			}
			$this->log('Removed file');
			$this->p->removeEntry($this->name);
			echo "File $this->name removed." . PHP_EOL;
			// this will prevent ArticleList::save() from writing this file
			unset($this->name);
			$this->needSave();
		}
		return true;
	}
	public function move($newname = '') {
	// renames a file
		if($this->isredirect() or ($this->name === $newname)) {
			return true;
		}
		if(!$newname or !is_string($newname)) while(true) {
			$newname = $this->getline(array(
				'initialtext' => $this->name,
				'prompt' => 'New name: ',
			));
			if($newname === 'q') return false;
			break;
		}
		// what to do if new filename already exists
		if($this->p->has($newname)) {
			echo 'New name already exists: ' . $newname . PHP_EOL;
			if($this->p->isredirect($newname)) {
				if(!$this->ynmenu('The existing file is a redirect. Do you want to overwrite it?')) {
					return false;
				}
			} else {
				return false;
			}
		}
		// fix any enclosings
		$enclosings = $this->p->bfind(array(
			'parent' => $this->name,
			'quiet' => true,
		));
		foreach($enclosings as $enclosing) {
			$enclosing->parent = $newname;
		}
		// change the name internally
		$oldname = $this->name;
		$oldpath = $this->path(array('type' => 'none'));
		$this->name = $newname;
		// move the physical file
		if($this->isfile() && ($newpath = $this->path(array('type' => 'none')))) {
			$this->shell(array('mv', array(
				'-n', $oldpath, $newpath
			)));
			$this->log('Moved file ' . $oldname);
		}
		// change the catalog
		if($this->p->has($oldname)) {
			$this->p->moveEntry($oldname, $this->name);
			// make redirect
			$this->p->addRedirect(array(
				'handle' => $oldname,
				'target' => $newname,
			));
		}
		return true;
	}
	public function effect_rename(array $paras) {
	// fix files that have been renamed in the catalog but not the library, or 
	// vice versa
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'searchlist' => 'List to search in for the existing version of 
					the file',
				'elist' => 'List that we found the lost file in',
				'domove' => 'Whether we need to move the physical file',
			),
			'errorifempty' => array('searchlist', 'elist'),
			'default' => array('domove' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$searchlist = $paras['searchlist'];
		$elist = $paras['elist'];
		echo PHP_EOL . "Type 'q' to quit and 'i' to get information about this file." . PHP_EOL;
		// ask user
		while(true) {
			$newname = $this->getline("New name of file: ");
			// get a way to escape
			if($newname === "q") {
				return false;
			} elseif($newname === "i") {
				$this->inform();
			} else {
				if(!isset($searchlist[$newname])) {
					echo "Could not find a file with this name; try again." . PHP_EOL;
					continue;					
				}
				$searchres = $searchlist[$newname];
				if(isset($elist[$searchres->name])) {
					echo "File already exists in catalog." . PHP_EOL;
					continue;
				}
				$this->needSave();
				$this->p->log("File " . $this->name . " renamed to " . $newname . " (check)." . PHP_EOL);
				if($paras['domove']) {
					$this->shell(array('mv', array(
						'-n',
						$this->path(array('type' => 'none')),
						$searchres->path(array('type' => 'none')),
					)));
				}
				$oldname = $this->name;
				$this->name = $searchres->name;
				$this->folder = $searchres->folder;
				$this->sfolder = $searchres->sfolder;
				$this->ssfolder = $searchres->ssfolder;
				$this->p->moveEntry($oldname, $this->name);
				// make redirect
				$this->p->addRedirect(array(
					'handle' => $oldname,
					'target' => $newname,
				));
				echo "Found new filename." . PHP_EOL;
				return true;
			}
		}
	}
	/*
	 * Types
	 */
	public static function typeToString($type) {
		switch($type) {
			case self::JOURNAL: return 'journal';
			case self::CHAPTER: return 'chapter';
			case self::BOOK: return 'book';
			case self::THESIS: return 'thesis';
			case self::WEB: return 'web';
			case self::MISCELLANEOUS: return 'miscellaneous';
			case self::REDIRECT: return 'redirect';
			case self::SUPPLEMENT: return 'supplement';
			default: throw new EHException($type);
		}
	}
	private static $stringToType = array(
		self::BOOK => self::BOOK,
		'book' => self::BOOK,
		'b' => self::BOOK,
		self::JOURNAL => self::JOURNAL,
		'journal' => self::JOURNAL,
		'j' => self::JOURNAL,
		self::CHAPTER => self::CHAPTER,
		'chapter' => self::CHAPTER,
		'c' => self::CHAPTER,
		self::THESIS => self::THESIS,
		'thesis' => self::THESIS,
		't' => self::THESIS,
		self::WEB => self::WEB,
		'web' => self::WEB,
		'w' => self::WEB,
		self::MISCELLANEOUS => self::MISCELLANEOUS,
		'miscellaneous' => self::MISCELLANEOUS,
		'misc' => self::MISCELLANEOUS,
		'm' => self::MISCELLANEOUS,
		self::REDIRECT => self::REDIRECT,
		'redirect' => self::REDIRECT,
		'r' => self::REDIRECT,
		self::SUPPLEMENT => self::SUPPLEMENT,
		'supplement' => self::SUPPLEMENT,
		's' => self::SUPPLEMENT,
	);
	// Returns a function that converts an arbitrary value to a type if possible
	public function getConverterToType(array $paras) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'excluded' => 'Types to exclude',
			),
			'default' => array('excluded' => array()),
			'checkparas' => array(
				'excluded' => function($in) {
					return is_array($in);
				}
			),
		))) return false;
		$arr = array();
		foreach(self::$stringToType as $key => $value) {
			if(!in_array($value, $paras['excluded'], true)) {
				$arr[$key] = $value;
			}
		}
		return function($in) use($arr) {
			if(isset($arr[$in])) {
				return $arr[$in];
			} else {
				return false;
			}
		};
	}
	// Returns an array of arrays, one for each type
	public function getTypeSynonyms(array $paras = array()) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'excluded' => 'Types to exclude',
			),
			'default' => array('excluded' => array()),
			'checkparas' => array(
				'excluded' => function($in) {
					return is_array($in);
				}
			),
		))) return false;
		$arr = array();
		foreach(self::$stringToType as $key => $value) {
			if(!in_array($value, $paras['excluded'], true)) {
				$arr[$value][] = $key;
			}
		}
		return $arr;
	}
	public function getTypeSynonymsAsString(array $paras = array()) {
		$arr = $this->getTypeSynonyms($paras);
		$out = '';
		foreach($arr as $key => $value) {
			$out .= self::typeToString($key) . ': ' . implode(', ', $value) 
				. PHP_EOL;
		}
		return $out;
	}

	/*
	 * Kinds of Article objects
	 */
	public function isor() {
	// whether the file belongs to any of the categories specified by the arguments
		$args = func_get_args();
		foreach($args as $arg) {
			// if input is invalid, this will throw an exception
			if($this->{'is' . $arg}()) {
				return true;
			}
		}
		return false;
	}
	abstract public function isfile();
	public function isnofile() {
		return !$this->isfile();
	}
	public function ispdf() {
		return $this->getNameParser()->extension() === 'pdf';
	}
	public function issupplement() {
		return $this->type === self::SUPPLEMENT;
	}
	public function isredirect() {
		return $this->type === self::REDIRECT;
	}
	public function resolve_redirect() {
		if($this->isredirect()) {
			return (string) $this->parent;
		} else {
			return $this->name();
		}
	}
	public function isthesis() {
		return $this->type === self::THESIS;
	}
	public function thesis_getuni() {
	// get the university for a thesis
		if(!$this->isthesis()) {
			return false;
		} else {
			return $this->publisher();
		}
	}
	public function thesis_gettype() {
		if(!$this->isthesis()) {
			return false;
		} else {
			return $this->series;
		}
	}
	public function supp_getbasic() {
		if($this->issupplement()) {
			return (string) $this->parent;
		} else {
			return false;
		}
	}
	public function isinpress() {
	// checks whether file is in "in press" (and therefore, year etcetera cannot be given)
		return ($this->start_page === 'in press');
	}
	public function isweb() {
	// is this a web publication?
		return $this->type === self::WEB;
	}
	private function getEnclosing() {
		if($this->parent) {
			return $this->p->get($this->parent);
		} else {
			return false;
		}
	}
	private function getEnclosingAuthors(array $paras = array()) {
		$obj = $this->getEnclosing();
		if($obj === false) {
			return false;
		}
		if(!$obj->authors) {
			return false;
		}
		if($obj->authors === $this->authors) {
			return false;
		}
		return $obj->getAuthors($paras);
	}

	/*
	 * Citing
	 */
	public function cite($asString = false) {
		return $this->__invoke($asString);
	}
	public function __invoke($asString = false) {
	// cite according to specified style
		if($this->issupplement()) {
			if($name = $this->supp_getbasic()) {
				return $this->p->cite($name);
			} else {
				return false;
			}
		}
		if(!$this->p->citetype) {
			$this->p->citetype = 'wp';
		}
		$func = 'cite' . $this->p->citetype;
		$result = $this->$func();
		if(is_array($result)) {
			$result = implode(PHP_EOL, $result);
		}
		return $result;
	}
	public function getAuthors(array $paras) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'separator' => 'Text between two authors',
				'lastSeparator' => 'Text between last two authors',
				'separatorWithTwoAuthors' => 
					'Text between authors if there are only two',
				'asArray' => 'Return authors as an array',
				'capitalizeNames' => 'Whether to capitalize names',
				'spaceInitials' => 'Whether to space initials',
				'initialsBeforeName' => 
					'Whether to place initials before the surname',
				'firstInitialsBeforeName' =>
					"Whether to place the first author's initials before their surname",
			),
			'default' => array(
				'separator' => ';',
				'asArray' => false,
				'capitalizeNames' => false,
				'spaceInitials' => false,
				'initialsBeforeName' => false,
				'firstInitialsBeforeName' => false,
			),
			'defaultfunc' => array(
				'lastSeparator' => function($paras) {
					return $paras['separator'];
				},
				'separatorWithTwoAuthors' => function($paras) {
					return $paras['lastSeparator'];
				}
			),
		))) return false;
		$array = $this->_getAuthors();
		if($paras['asArray']) {
			return $array;
		}
		$out = '';
		foreach($array as $key => $a) {
			// Separators
			if($key > 0) {
				if(isset($array[$key + 1])) {
					$out .= $paras['separator'] . ' ';
				} elseif($key === 1) {
					$out .= $paras['separatorWithTwoAuthors'] . ' ';
				} else {
					$out .= $paras['lastSeparator'] . ' ';
				}
			}
			// Process author
			if($paras['capitalizeNames']) {
				$a[0] = mb_strtoupper($a[0]);
			}
			if($paras['spaceInitials']) {
				if(isset($a[1])) {
					$a[1] = trim(str_replace('.', '. ', $a[1]));
				}
			}
			if(isset($a[1])) {
				if(($key === 0) ? $paras['firstInitialsBeforeName'] : $paras['initialsBeforeName']) {
					$author = $a[1] . ' ' . $a[0];
				} else {
					$author = $a[0] . ', ' . $a[1];
				}
				if(isset($a[2])) {
					$author .= ', ' . $a[2];
				}
			} else {
				$author = $a[0];
			}
			$out .= $author;
		}
		return $out;
	}
	public static function explodeAuthors($in) {
		$authors = explode('; ', $in);
		return array_map(function($author) {
			return explode(', ', $author);
		}, $authors);
	}
	/*
	 * Should return array like:
	 * array(
	 *		0 => array('Zijlstra', 'J.S.'),
	 *		1 => array('Smith", 'J.', 'Jr.'),
	 * );
	 */
	abstract protected function _getAuthors();
	public function citepaper() {
	// like citenormal(), but without WP style links and things
		return $this->citenormal(false);
	}
	public function citenormal($mw = true) {
	// cites according to normal WP citation style
	// if $mw = false, no MediaWiki markup is used
		// this is going to be the citation
		if($mw) {
			$out = '*';
		} else {
			$out = '';
		}
		// replace last ; with ", and"; others with ","
		$out .= $this->getAuthors(array(
			'separator' => ',',
			'lastSeparator' => ' and',
		));
		$out .= ' ' . $this->year. '. ';
		if($mw) {
			$url = $this->geturl();
			if($url !== false) {
				$out .= "[" . $url . " ";
			}
		}
		$out .= $this->title;
		// TODO: guess whether "subscription required" is needed based on URL
		if($mw and ($url !== false)) {
			$out .= "] (subscription required)";
		}
		$out .= ". ";
		switch($this->type) {
			case self::JOURNAL:
				// journals (most common case)
				$out .= $this->journal() . " ";
				if(strlen($this->series) > 0) {
					// need to catch "double series"
					$out .= "(" . preg_replace("/;/", ") (", $this->series) . ")";
				}
				$out .= $this->volume;
				if(strlen($this->issue) > 0) {
					$out .= "($this->issue)";
				}
				$out .= ':' . $this->pages() . '.';
				break;
			case self::CHAPTER:
				if($this->start_page === $this->end_page) {
					$out .= "P. $this->start_page in ";
				} else {
					$out .= "Pp. " . $this->start_page . "–" . $this->end_page . " in ";
				}
				$out .= $this->getEnclosingAuthors(array(
					'separator' => ',',
					'lastSeparator' => ' and',
				));
				$out .= " (eds.). ";
				$enclosing = $this->getEnclosing();
				$out .= $enclosing->title . ". ";
				$out .= $enclosing->publisher();
				if(strlen($enclosing->pages) > 0) {
					$out .= ", " . $enclosing->pages . " pp";
				}
				$out .= ".";
				break;
			case self::BOOK:
				$out .= ' ' . $this->publisher();
				if(strlen($this->pages) > 0) {
					$out .= ", " . $this->pages . " pp";
				}
				$out .= ".";
				break;
		}
		if(!$mw && (($doi = $this->doi()) !== false)) {
			$out .= ' doi:' . $doi;
		}
		// final cleanup
		$out = preg_replace(array("/\s\s/", "/\.\./"), array(" ", "."), $out);
		if($mw) {
			$out = wikify($out);
		}
		return $out;
	}
	public function citewp() {
	// cites according to {{cite journal}} etc.
		/*
		 * stuff related to {{cite doi}} and friends
		 */
		// determines whether only one citation is returned or two if 
		// {{cite doi}} or friends can be used
		$verbosecite = $this->p->verbosecite;
		if(($doi = $this->doi()) !== false) {
			// to fix bug 28212. Commented out for now since it seems we don't
			// need it. Or perhaps we do; I never know.
			$doi = str_replace(array('<' /*, '>' */), array('.3C' /*, '.3E' */), $this->doi);
		}
		$out1 = '';
		if($doi !== false) {
			// {{cite doi}}
			$out1 = "{{cite doi|" . $doi . "}}";
		} elseif(($jstor = $this->jstor()) !== false) {
			// {{cite jstor}}
			$out1 = "{{cite jstor|" . $jstor . "}}";
		} elseif(($hdl = $this->hdl()) !== false) {
			// {{cite hdl}}
			$out1 = "{{cite hdl|" . $hdl . "}}";
		}
		if($verbosecite === false && $out1 !== '') {
			return $out1;
		}
		switch($this->type) {
			case self::JOURNAL: 
				$temp = 'journal'; 
				break;
			case self::BOOK: 
			case self::CHAPTER: 
				$temp = 'book'; 
				break;
			case self::THESIS: 
				$temp = 'thesis'; 
				break;
			case self::MISCELLANEOUS: 
				if($out1) {
					return $out1;
				} else {
					return false;
				}
			case self::WEB: 
				$temp = 'web'; 
				break;
			default:
				throw new EHException('Unrecognized class');
		}
		$paras = array();
		// authors
		$authors = $this->getAuthors(array('asArray' => true));
		foreach($authors as $key => $author) {
			if($key < 9) {
			// templates only support up to 9 authors
				$paras['last' . ($key + 1)] = $author[0];
				if(isset($author[1])) {
					$paras['first' . ($key + 1)] = $author[1];
				}
			} else {
				if(!isset($paras['coauthors'])) {
					$paras['coauthors'] = '';
				}
				$paras['coauthors'] .= implode(', ', $author) . '; ';
			}
		}
		if(isset($paras['coauthors'])) {
			$paras['coauthors'] = preg_replace('/; $/u', '', $paras['coauthors']);
		}
		// easy stuff we need in all classes
		$paras['year'] = $this->year;
		if(($hdl = $this->hdl()) !== false) {
			$paras['id'] = '{{hdl|' . $hdl . '}}';
		}
		$paras['jstor'] = $this->getIdentifier('jstor');
		$paras['pmid'] = $this->getIdentifier('pmid');
		$paras['url'] = $this->getIdentifier('url');
		$paras['doi'] = ($doi !== false) ? $doi : '';
		$paras['pmc'] = $this->getIdentifier('pmc');
		$paras['publisher'] = $this->publisher();
		$paras['location'] = $this->location();
		$paras['isbn'] = $this->getIdentifier('isbn');
		$paras['pages'] = $this->pages();
		switch($this->type) {
			case self::JOURNAL:
				$paras['title'] = $this->title;
				$paras['journal'] = $this->journal;
				$paras['volume'] = $this->volume;
				$paras['issue'] = $this->issue;
				break;
			case self::BOOK:
				$paras['title'] = $this->title;
				if(!$paras['pages']) {
					$paras['pages'] = $this->pages;
				}
				$paras['edition'] = $this->edition;
				break;
			case self::CHAPTER:
				$paras['chapter'] = $this->title;
				$enclosing = $this->getEnclosing();
				$paras['title'] = $enclosing->title;
				$paras['publisher'] = $enclosing->publisher();
				$paras['location'] = $enclosing->location();
				$paras['edition'] = $this->edition;
				$bauthors = $this->getEnclosingAuthors(array(
					'asArray' => true,
				));
				if($bauthors !== false) {
					foreach($bauthors as $key => $author) {
					// only four editors supported
						if($key < 4) {
							$paras['editor' . ($key + 1) . '-last'] = 
								$author[0];
							$paras['editor' . ($key + 1) . '-first'] = 
								$author[1];
						} else {
						// because cite book only supports four editors, we have to hack by putting the remaining editors in |editor4-last=
							if($key === 4) {
								unset($paras['editor4-first']);
								$paras['editor4-last'] = 
									implode(', ', $bauthors[3]) . '; ';
							}
							$paras['editor4-last'] .= 
								implode(', ', $author) . '; ';
						}
					}
					// double period bug
					if(isset($paras['editor4-last']) and strpos($paras['editor4-last'], ';') !== false) {
						$paras['editor4-last'] = preg_replace(
							array('/; $/u', '/\.$/u'), 
							array('', ''), 
							$paras['editor4-last']
						);
					} else {
						$paras['editor' . count($bauthors) . '-first'] = 
							preg_replace('/\.$/u', '', 
								$paras['editor' . ($key + 1) . '-first']
							);
					}
				}
				break;
			case self::THESIS:
				$paras['title'] = $this->title;
				$paras['degree'] = $this->thesis_gettype();
				$paras['publisher'] = $this->thesis_getuni();
				$paras['pages'] = $this->pages;
				break;
			case self::WEB:
				$paras['title'] = $this->title;
				$paras['publisher'] = $this->publisher();
				break;
		}
		if($this->p->includerefharv) {
			$paras['ref'] = 'harv';
		}
		$out = $sfn = '';
		if($this->p->includesfn) {
			$out = $sfn = '<!--' . $this->getsfn() . '-->';
		}
		$out .= '{{cite ' . $temp;
		foreach($paras as $key => $value) {
			if($value !== NULL && $value !== '' && $value !== false) {
				$out .= ' | ' . $key . ' = ' . $value;
			}
		}
		$out .= '}}';
		// final cleanup
		$out = preg_replace(
			array("/\s\s/u", "/(?<!\.)\.\.(?!\.)/u"), array(" ", "."), wikify($out)
		);
		return ($verbosecite and $out1) ? array($sfn . $out1, $out) : $out;
	}
	public function getsfn() {
		$sfn = '{{Sfn|';
		$auts = $this->getAuthors(array('asArray' => true));
		for($i = 0; $i < 4; $i++) {
			if(isset($auts[$i])) {
				$sfn .= $auts[$i][0] . '|';
			}
		}
		$sfn .= $this->year . '}}';
		return $sfn;
	}
	public function citelemurnews() {
		$authors = $this->getAuthors(array());
		switch($this->type) {
			case self::JOURNAL:
				$out = $authors . '. ' .
					$this->year . '. ' .
					$this->title . '. ' .
					$this->journal() . ' ' .
					$this->volume . ': ' .
					$this->start_page . '–' .
					$this->end_page . '.';
				break;
			case self::BOOK:
				$out = $authors . '. ' .
					$this->year . '. ' .
					$this->title . '. ' .
					$this->publisher() . ', ' .
					$this->location() . '. ';
				break;
		}
		// TODO: support non-journal citations
/*
Ranaivoarisoa, J.F.; Ramanamahefa, R.; Louis, Jr., E.E.; Brenneman, R.A. 2006. Range extension of Perrier’s sifaka, <i>Propithecus perrieri</i>, in the Andrafiamena Classified Forest. Lemur News 11: 17-21.

Book chapter
Ganzhorn, J.U. 1994. Les lémuriens. Pp. 70-72. In: S.M. Goodman; O. Langrand (eds.). Inventaire biologique; Forêt de Zombitse. Recherches pour le Développement, Série Sciences Biologiques, n° Spécial. Centre d’Information et de Documentation Scientifique et Technique, Antananarivo, Madagascar.

Book
Mittermeier, R.A.; Konstant, W.R.; Hawkins, A.F.; Louis, E.E.; Langrand, O.; Ratsimbazafy, H.J.; Rasoloarison, M.R.; Ganzhorn, J.U.; Rajaobelina, S.; Tattersall, I.; Meyers, D.M. 2006. Lemurs of Madagascar. Second edition. Conservation International, Washington, DC, USA.

Thesis
Freed, B.Z. 1996. Co-occurrence among crowned lemurs (<i>Lemur coronatus</i>) and Sanford’s lemur (<i>Lemur fulvus sanfordi</i>) of Madagascar. Ph.D. thesis, Washington University, St. Louis, USA.

Website
IUCN. 2008. IUCN Red List of Threatened Species. <www.iucnredlist.org>. Downloaded on 21 April 2009.
*/
		// final cleanup
		$out = preg_replace(array("/\s\s/", "/\.\./"), array(" ", "."), $out);
		return $out;
	}
	public function citebzn() {
	// cites according to BZN citation style
		// replace last ; with " &"; others with ","
		$out = '<b>';
		$out .= $this->getAuthors(array(
			'separator' => ',',
			'lastSeparator' => ' &',
		));
		$out .= '</b> ' . $this->year . '. ';
		switch($this->type) {
			case self::JOURNAL:
				$out .= $this->title;
				$out .= '. ';
				$out .= '<i>' . $this->journal() . '</i>, ';
				if($this->series) {
					// need to catch "double series"
					$out .= "(" . str_replace(";", ") (", $this->series) . ")";
				}
				$out .= '<b>' . $this->volume . '</b>: ';
				$out .= $this->pages() . '.';
				break;
			case self::CHAPTER:
				$enclosing = $this->getEnclosing();
				$out .= $this->title . '. <i>in</i> ';
				$out .= str_replace("(Ed", '(ed', $this->getEnclosingAuthors());
				$out .= ', <i>' . $enclosing->title . '</i>. ' 
					. $enclosing->pages . ' pp. ' . $enclosing->publisher();
				if($enclosing->location()) {
					$out .= ', ' . $enclosing->location();
				}
				$out .= '.';
				break;
			case self::BOOK:
				$out .= '<i>' . $this->title . '.</i>';
				if($this->pages) {
					$out .= ' ' . $this->pages . ' pp.';
				}
				$out .= ' ' . $this->publisher();
				if($this->location()) {
					$out .= ', ' . $this->location();
				}
				$out .= '.';
				break;
		}
		// final cleanup
		$out = str_replace(array("  ", ".."), array(" ", "."), $out);
		return $out;
	}
	public function citepalaeontology() {
		// this is going to be the citation
		$out = '';
		
		$authorsParas = array(
			'capitalizeNames' => true,
			'spaceInitials' => true,
			'separator' => ',',
			'lastSeparator' => ' and',
		);
		$out .= $this->getAuthors($authorsParas);
		$out .= ' ' . $this->year . '. ';
		switch($this->type) {
			case self::JOURNAL:
				$out .= $this->title . '. <i>' . $this->journal() . '</i>, ';
				// TODO: series
				$out .= '<b>' . $this->volume . '</b>, ' . $this->pages();
				break;
			case self::BOOK:
				$out .= '<i>' . $this->title . '.</i> ';
				$out .= $this->publisher() . ', ';
				if($this->location()) {
					$out .= $this->location() . ', ';
				}
				$out .= $this->pages . ' pp.';
				break;
			case self::CHAPTER:
				$enclosing = $this->getEnclosing();
				$out .= $this->title . '. <i>In</i> ';
				$out .= $this->getEnclosingAuthors($authorsParas);
				$out .= ' (eds). ' . $enclosing->title . '.</i> ';
				$out .= $enclosing->publisher() . ', ';
				if($enclosing->location()) {
					$out .= $enclosing->location() . ', ';
				}
				$out .= $enclosing->pages . ' pp.';
				break;
			case self::THESIS:
				$out .= '<i>' . $this->title . '</i>. Unpublished ';
				switch($this->thesis_gettype()) {
					case 'PhD': $out .= 'Ph.D.'; break;
					case 'MSc': $out .= 'M.Sc.'; break;
					case 'BSc': $out .= 'B.Sc.'; break;
				}
				$out .= ' thesis, ' . $this->thesis_getuni();
				$out .= ', ' . $this->pages . ' pp.';
				break;
			default:
				$out .= $this->title . '. ';
				$out .= '<!--Unknown citation type; fallback citation-->';
				break;
		}
		// final cleanup
		$out .= ".";
		$out = preg_replace(array("/\s\s/u", "/\.\./u"), array(" ", "."), $out);
		return $out;
	}
	public function citejpal() {
		// this is going to be the citation
		$out = '';
		$out .= $this->getAuthors(array(
			'capitalizeNames' => true,
			'initialsBeforeName' => true,
			'separator' => ',',
			'lastSeparator' => ', and',
			'separatorWithTwoAuthors' => ' and',
			'spaceInitials' => true,
		));
		$out .= ' ' . $this->year . '. ' . $this->title;
		switch($this->type) {
			case self::JOURNAL:
				$out .= '. ' . $this->journal() . ', ';
				if($this->series) {
					$out .= 'ser. ' . $this->series . ', ';
				}
				$out .= $this->volume . ':' . $this->pages();
				break;
			case self::CHAPTER:
				$out .= ', ' . $this->pages() . '. <i>In</i> ';
				$bauthors = $this->getEnclosingAuthors(array(
					'capitalizeNames' => true,
					'firstInitialsBeforeName' => true,
					'initialsBeforeName' => true,
					'separator' => ',',
					'lastSeparator' => ', and',
					'separatorWithTwoAuthors' => ' and',
					'spaceInitials' => true,
				));
				$out .= $bauthors;
				if(strpos($bauthors, ' and ') !== false) {
					$out .= ' (eds.), ';
				} else {
					$out .= ' (ed.), ';
				}
				$out .= $this->getEnclosing()->title;
			case self::BOOK:
				$out .= '. ' . $this->publisher();
				if($this->location()) {
					$out .= ', ' . $this->location();
				}
				if($this->pages) {
					$out .= ', ' . $this->pages . ' p.';
				}
				break;
			default:
				$out .= $this->title . '. ';
				$out .= '<!--Unknown citation type; fallback citation-->';
				break;
		}
		// final cleanup
		$out .= '.';
		$out = preg_replace(
			array("/\s\s/u", "/\.\./u"), 
			array(" ", "."), 
			$out
		);
		return $out;
	}
	public function citebibtex() {
		// lambda function to add a property to the output
		$add = function($key, $value, $mandatory = false) use(&$out) {
			if($value === NULL) {
				if($mandatory) {
					echo 'Bibtex error: required property ' . $key 
						. ' is empty' . PHP_EOL;
				}
				return;
			}
			$out .= "\t" . $key . ' = "' . $value . "\",\n";
		};
		$out = '@';
		switch($this->type) {
			case self::JOURNAL:
				$out .= 'article';
				break;
			case self::BOOK:
				$out .= 'book';
				break;
			case self::CHAPTER:
				$out .= 'incollection';
				break;
			case self::THESIS:
				switch($this->thesis_gettype()) {
					case 'PhD': $out .= 'phdthesis'; break;
					case 'MSc': $out .= 'mscthesis'; break;
					default: $out .= 'misc'; break;
				}
				break;
			default:
				$out .= 'misc';
				break;
		}
		$out .= '{' . $this->getrefname() . ",\n";
		$authors = $this->getAuthors(array(
			'spaceInitials' => true,
			'separator' => ' and',
		));
		// stuff that goes in every citation type
		$add('author', $authors, true);
		$add('year', $this->year, true);
		$title = '{' . str_replace(
			array('<i>', '</i>'),
			array('\textit{', '}'),
			$this->title
		) . '}';
		$add('title', $title, true);
		switch($this->type) {
			case self::THESIS:
				$add('school', $this->thesis_getuni(), true);
				break;
			case self::JOURNAL:
				$add('journal', $this->journal(), true);
				$add('volume', $this->volume);
				$add('number', $this->issue);
				$add('pages', $this->start_page . '--' . $this->end_page);
				break;
			case self::BOOK:
				$add('publisher', $this->publisher(), true);
				$add('address', $this->location());
				break;
		}
		if(($isbn = $this->getIdentifier('isbn')) !== false) {
			$add('note', '{ISBN} ' . $isbn);
		}
		$out .= '}';
		return $out;
	}
	public function citezootaxa() {
		// this is going to be the citation
		$out = '';
		$out .= $this->getAuthors(array(
			'separator' => ',',
			'lastSeparator' => ' &',
		));
		$out .= ' (' . $this->year . ') ';
		switch($this->type) {
			case self::JOURNAL:
				$out .= $this->title . '. <i>' . $this->journal() . '</i>, ';
				$out .= $this->volume . ', ' . $this->pages();
				break;
			case self::CHAPTER:
				$out .= $this->title . '. <i>In</i>: ';
				$out .= $this->getEnclosingAuthors(array(
					'separator' => ',',
					'lastSeparator' => ' &',					
				));
				$enclosing = $this->getEnclosing();
				$out .= ' (Eds), <i>' . $enclosing->title . '</i>. ';
				$out .= $enclosing->publisher() . ', ' . $enclosing->location();
				$out .= ', pp. ' . $this->pages();
				break;
			case self::BOOK:
				$out .= '<i>' . $this->title . '</i>. ' . $this->publisher();
				if($this->location()) {
					$out .= ', ' . $this->location();
				}
				if($this->pages) {
					$out .= ', ' . $this->pages. ' pp.';
				}
				break;
			default:
				$out .= $this->title . '. ';
				$out .= "<!--Unknown citation type; fallback citation-->";
				break;
		}
		// final cleanup
		$out .= ".";
		$out = preg_replace(array("/\s\s/u", "/\.\./u"), array(" ", "."), $out);
		return $out;
	}
	public function echocite($paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'mode' =>
					'Citation style to be used. If not set, the catalog\'s default style is used.',
			),
			'default' => array(
				'mode' => '',
			),
			'checkparas' => array(
				'mode' => function($in) {
					return method_exists($this, 'cite' . $in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->p->verbosecite = true;
		$cite = $this->{'cite' . $paras['mode']}();
		echo PHP_EOL;
		if(is_array($cite)) {
			foreach($cite as $cit) {
				echo $cit . PHP_EOL;
			}
		} else {
			echo $cite . PHP_EOL;
		}
		return true;
	}
	public function get_citedoiurl($var = 'doi') {
	// returns URL to Wikipedia cite doi-family template for this Article
	// by default uses cite doi; by setting var to something else, jstor and a few others can be set
		if($var === 'jst') {
			// due to limitations in Parser
			$var = 'jstor';
		}
		$id = $this->getIdentifier($var);
		if($id === false) {
			return false;
		} else {
			return 'https://en.wikipedia.org/w/index.php?action=edit&title=' 
				. 'Template:Cite_' . $var . '/' . $id;
		}
	}
	public function getharvard($paras = array()) {
	// get a Harvard citation
		// TODO: implement getting both Zijlstra et al. (2010) and (Zijlstra et al., 2010)
		// TODO: more citetype dependence
		$authors = $this->getAuthors(array('asArray' => true));
		$out = '';
		switch(count($authors)) {
			case 0:
				return ''; // incomplete info
			case 1:
				$out .= $authors[0][0];
				break;
			case 2:
				$out .= $authors[0][0] . ' and ' . $authors[1][0];
				break;
			default:
				$out .= $authors[0][0];
				switch($this->p->citetype) {
					case 'jpal':
						$out .= ' <i>et al.</i>';
						break;
					case 'paper': case 'normal': default:
						$out .= ' et al.';
						break;
				}
				break;
		}
		$out .= ' (' . $this->year . ')';
		return $out;
	}
	private function pages() {
	// return a string representing the pages of the article
		if(strlen($this->start_page) > 0) {
			if(strlen($this->end_page) > 0) {
				if($this->start_page === $this->end_page) {
					// single page
					return (string) $this->start_page;
				} else {
					// range
					return $this->start_page . '–' . $this->end_page;
				}
			} else {
				return (string) $this->start_page;
			}
		} else {
			return false;
		}
	}
	/*
	 * Manual editing
	 */
	public function edittitle(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// function to create the internal title array
		$makeSplit = function(/* string */ $title) {
			$splitTitle = explode(' ', $title);
			foreach($splitTitle as $key => $word) {
				echo $key . ': ' . $word . PHP_EOL;
			}
			return $splitTitle;
		};
		// and another to convert it back into a good title
		$unite = function(array $splitTitle) {
			$title = implode(' ', $splitTitle);
			$title = preg_replace('/^\s+|\s+$|\s+(?= )/u', '', $title);
			return $title;
		};
		// smartly convert a word to lowercase
		$tolower = 'mb_strtolower';
		// and uppercase
		$toupper = function(/* string */ $word) {
			if(isset($word[1]) && $word[0] === '(') {
				$word[1] = mb_ucfirst($word[1]);
			} else {
				$word = mb_ucfirst($word);
			}
			return $word;
		};
		$looper = function($f) use(&$splitTitle) {
			return function($cmd, array $data) use(&$splitTitle, $f) {
				if(count($data) !== 2) {
					echo 'Invalid argument' . PHP_EOL;
					return true;
				}
				for($i = $data[0]; $i <= $data[1]; $i++) {
					$splitTitle[$i] = $f($splitTitle[$i]);
				}
				return true;
			};
		};
		echo 'Current title: ' . $this->title . PHP_EOL;
		// the array to hold the title
		$splitTitle = $makeSplit($this->title);
		$this->menu(array(
			'prompt' => 'edittitle> ',
			'options' => array(
				'l' => 'Make a word lowercase',
				'u' => 'Make a word uppercase',
				'i' => 'Italicize a word',
				'e' => 'Edit an individual word',
				't' => 'Merge a word with the next word',
				'r' => 'Remove a word',
				'v' => 'Smartly divide an individual word',
				'w' => 'Edit the whole title',
				'p' => 'Preview the edited title',
				'c' => 'Recalculate the words',
				'o' => 'Open this file',
				'f' => 'Edit this file',
				'a' => 'Quit this file without saving changes',
				's' => 'Save the changed title',
			),
			'processcommand' => function($cmd, &$data) use(&$splitTitle) {
				$cmd = trim($cmd);
				if(preg_match('/^[a-z]$/', $cmd)) {
					$data = array();
					return $cmd;
				} elseif(preg_match('/^([a-z])\s*(\d+)\s*-\s*(\d+)\s*$/u', $cmd, $matches)) {
					$beg = (int) $matches[2];
					$end = (int) $matches[3];
					if($beg > $end) {
						echo 'Range invalid: beginning > end' . PHP_EOL;
						return false;
					} elseif(!isset($splitTitle[$end])) {
						echo 'Range invalid: no word ' . $end . PHP_EOL;
						return false;
					} elseif(!isset($splitTitle[$beg])) {
						echo 'Range invalid: no word ' . $beg . PHP_EOL;
						return false;
					}
					$data = array($beg, $end);
					return $matches[1];
				} elseif(preg_match('/^([a-z])\s*(\d+)$/', $cmd, $matches)) {
					$n = (int) $matches[2];
					if(!isset($splitTitle[$n])) {
						echo 'Range invalid: no word ' . $n . PHP_EOL;
						return false;
					}
					$data = array($n, $n);
					return $matches[1];
				} else {
					return false;
				}
			},
			'process' => array(
				'l' => $looper($tolower),
				'u' => $looper($toupper),
				'i' => function($cmd, array $data) use(&$splitTitle) {
					if(count($data) !== 2) {
						echo 'Invalid argument' . PHP_EOL;
						return true;
					}
					$splitTitle[$data[0]] = '<i>' . $splitTitle[$data[0]];
					$splitTitle[$data[1]] .= '</i>';
					return true;				
				},
				'e' => function($cmd, array $data) use(&$splitTitle) {
					if(count($data) !== 2 || $data[0] !== $data[1]) {
						echo 'Invalid argument' . PHP_EOL;
						return true;
					}
					echo 'Current value of word ' . $data[0] . ': ' 
						. $splitTitle[$data[0]] . PHP_EOL;
					$splitTitle[$data[0]] = $this->getline('New value: ');
				},
				't' => function($cmd, array $data) use(&$splitTitle) {
					if(count($data) !== 2 || $data[0] !== $data[1]) {
						echo 'Invalid argument' . PHP_EOL;
						return true;
					}
					$n = $data[0];
					$splitTitle[$n] .= $splitTitle[$n + 1];
					$splitTitle[$n + 1] = '';
					return true;
				},
				'r' => $looper(function() {
					return '';
				}),
				'v' => $looper(function($in) {
					return preg_replace(
						array(
							'/(?<=[a-z,\.\)])(?=[A-Z])/u', '/(?=\()/u', 
							'/(?<=,)(?=[a-zA-Z])/u'
						),
						array(' ', ' ', ' '),
						$in);
				}),
				'w' => function() use(&$splitTitle, $unite, $makeSplit) {
					$ret = $this->editWholeTitle(array(
						'new' => $unite($splitTitle),
					));
					if($ret === false) {
						return false;
					} else {
						$splitTitle = $makeSplit($this->title);
						return true;
					}
				},
				'p' => function() use($unite, &$splitTitle) {
					echo $unite($splitTitle) . PHP_EOL;
					return true;
				},
				'c' => function() use($makeSplit, $unite, &$splitTitle) {
					$splitTitle = $makeSplit($unite($splitTitle));
					return true;
				},
				'o' => function() {
					$this->openf();
					return true;
				},
				'f' => function() {
					$this->edit();
					return true;
				},
				'a' => function() {
					return false;
				},
				's' => function() use($unite, &$splitTitle) {
					$this->title = $unite($splitTitle);
					$this->format();
					echo 'New title: ' . $this->title . PHP_EOL;
					$this->log('Edited title');
					$this->needSave();
					return false;
				},
			),
		));
	}
	private function editWholeTitle(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'new' => 'New title to start with',
			),
			'default' => array(
				'new' => $this->title,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$newTitle = $paras['new'];
		echo 'Current title: ' . $newTitle . PHP_EOL;
		$options = array(
			'r' => 'Save new title, return to word-by-word editing',
			'b' => 'Do not save title, return to word-by-word editing',
			's' => 'Save new title',
			'a' => 'Do not save new title',
			'p' => 'Preview title',
			'e' => 'Edit title',
		);
		return $this->menu(array(
			'prompt' => 'editWholeTitle> ',
			'options' => $options,
			'processcommand' => function($cmd, &$data) use($options) {
				if(array_key_exists($cmd, $options)) {
					return $cmd;
				} else {
					// else pretend it is the 'e' command
					$data = $cmd;
					return 'e';
				}
			},
			'process' => array(
				'r' => function(&$cmd) use(&$newTitle) {
					$this->log('Edited title');
					$this->title = $newTitle;
					$cmd = true;
					return false;
				},
				'b' => function(&$cmd) {
					$cmd = true;
					return false;
				},
				's' => function(&$cmd) use(&$newTitle) {
					$this->log('Edited title');
					$this->title = $newTitle;
					$cmd = false;
					return false;				
				},
				'a' => function(&$cmd) {
					$cmd = false;
					return false;
				},
				'p' => function() use(&$newTitle) {
					echo $newTitle . PHP_EOL;
					return true;
				},
				'e' => function($cmd, $data) use(&$newTitle) {
					if($data === NULL) {
						$data = $this->getline('New title: ');
					}
					$newTitle = $data;
					return true;
				},
			),
		));
	}
	/*
	 * Adding data
	 */
	public function newadd(array $paras) {
	// add from ArticleList::newcheck(). This function gets the path and moves the file into the library.
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array('lslist' => 'List of files found using ls'),
			'errorifempty' => array('lslist'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$renameFunction = function() {
			$oldname = $this->name;
			$newname = $this->getline('New name: ');
			if($newname === 'q') {
				break;
			}
			// allow renaming to existing name, for example to replace in-press files, but warn
			if($this->p->has($newname)) {
				echo 'Warning: file already exists' . PHP_EOL;
			}
			$this->name = $newname;
			$this->shell(array(
				'cmd' => 'mv',
				'arg' => array(
					TEMPPATH . '/' . $oldname, 
					TEMPPATH . '/' . $newname
				),
			));
			return true;
		};
		$parser = new NameParser($this->name);
		if($parser->errorOccurred()) {
			$parser->printErrors();
			$cmd = $this->ynmenu(
				'This filename could not be parsed. Do you want to rename it?');
			if($cmd) {
				$renameFunction();
			}
		}
		switch($this->menu(array(
			'head' => 'Adding file ' . $this->name,
			'options' => array(
				'o' => 'open this file',
				'q' => 'quit',
				's' => 'skip this file',
				'n' => 'move this file to "Not to be cataloged"',
				'r' => 'rename this file',
				'' => 'add this file to the catalog',				
			),
			'process' => array(
				'o' => function() {
					$this->openf(array('place' => 'temp'));
					return true;
				},
				'r' => $renameFunction,
				'n' => function() {
					$this->shell(array(
						'cmd' => 'mv',
						'arg' => array(
							TEMPPATH . '/' . $this->name,
							TEMPPATH . '/Not to be cataloged/' . $this->name,
						),
					));
					return false;
				},
				'q' => function() {
					throw new StopException('newadd');
				}
			),
		))) {
			case 'n': case 's': return true;
			default: break;
		}
		/*
		 * get data
		 */
		// the next line may "rename" the file, but we do not actually rename it
		// until we move it to the repository, so remember the old name
		$oldname = $this->name;
		if(!$this->checkForExistingFile($paras['lslist'])) {
			return false;
		}
		if(!$this->determinePath()) {
			echo 'Unable to determine folder' . PHP_EOL;
			return false;
		}
		$this->shell(array(
			'cmd' => 'mv',
			'arg' => array(
				'-n',
				TEMPPATH . '/' . $oldname,
				$this->path(array('type' => 'none')),
			),
		));
		return $this->add();
	}
	protected /* bool */ function determinePath() {
		// short-circuiting
		return $this->fullPathSuggestions()
			or $this->folderSuggestions();
	}
	private /* bool */ function fullPathSuggestions() {
		if(!$this->p->sugglist) {
			$this->p->build_sugglist();
		}
		$key = $this->getkey();
		if(isset($this->p->sugglist[$key])) {
			$suggs = $this->p->sugglist[$key]->getsugg();
			foreach($suggs as $sugg) {
				$sugg = array_pad($sugg, 3, '');
				echo 'Suggested placement: ' . implode(' -> ', $sugg) . PHP_EOL;
				$cmd = $this->menu(array(
					'options' => array(
						'y' => 'this suggestion is correct',
						'n' => 'this suggestion is not correct',
						's' => 'stop suggestions',
						'q' => 'quit this file',
					),
				));
				switch($cmd) {
					case 'y':
						$this->setPathFromArray($sugg);
						return true;
					case 'n': break;
					case 's': return false;
				}
			}
		}
		return false;
	}
	private /* bool */ function folderSuggestions() {
		$sugg_lister = function($in) {
		// in folder suggestions part 2, print a list of suggestions and return useful array
		// input: array of folders
			if(!is_array($in)) {
				return;
			}
			// input is array with key: name of folder; value: array with contents
			// discard value, use key as value for out array
			foreach($in as $key => $value) {
				$out[] = $key;
			}
			foreach($out as $key => $value) {
				// print new keys (ints) to be used in user input
				echo $key . ': ' . $value . '; ';
			}
			echo PHP_EOL;
			return $out;
		};
		if(!$this->p->foldertree) {
			$this->p->build_foldertree();
		}
		$foldertree = $this->p->foldertree;
		$suggs = array();
		$menuOptions = array(
			'head' => 'Folder: ',
			'headasprompt' => true,
			'options' => array(
				'q' => 'Quit',
				'o' => 'Open this file',
			),
			'process' => array(
				'o' => function() {
					$this->openf(array('place' => 'temp'));
					return true;
				},
			),
			'processcommand' => function($cmd) use(&$suggs) {
				if(is_numeric($cmd) && isset($suggs[$cmd])) {
					return $suggs[$cmd];
				} else {
					return $cmd;
				}
			},
			'validfunction' => function($cmd) use(&$foldertree) {
				return ($cmd === '') || isset($foldertree[$cmd]);
			},
		);
		$path = array();
		for( ; count($foldertree) !== 0; ) {
			echo 'Suggestions: ';
			$suggs = $sugg_lister($foldertree);
			$path[] = $folder = $this->menu($menuOptions);
			if(isset($foldertree[$folder])) {
				$foldertree = $foldertree[$folder];
			} else {
				break;
			}
		}
		$this->setPathFromArray($path);
		return true;
	}
	abstract protected function setPathFromArray(array $path);
	abstract protected function setCurrentDate();
	private /* bool */ function checkForExistingFile($lslist) {
		if(!isset($lslist[$this->name])) {
			return true;
		}
		$options = array(
			'r' => 'move over the existing file',
			'd' => 'delete the new file',
			'o' => 'open the new and existing files',
		);
		$cmd = $this->menu(array(
			'head' => 'A file with this name already exists. Please enter a new filename',
			'options' => $options,
			'validfunction' => function() { return true; },
			'processcommand' => function($cmd, &$data) use($options) {
				if(!array_key_exists($cmd, $options)) {
					$data = $cmd;
					$cmd = 's';
				}
				return $cmd;
			},
			'process' => array(
				'o' => function() use($lslist) {
					$this->openf(array('place' => 'temp'));
					$lslist[$this->name]->openf();
					return true;
				},
				'r' => function() use($lslist) {
					$this->shell(array(
						'cmd' => 'mv',
						'arg' => array(
							TEMPPATH . '/' . $this->name,
							$lslist[$this->name]->path(array(
								'type' => 'none'
							)),
						),
					));
					$this->p->edit($this->name);
					return false;
				},
				'd' => function() {
					$this->shell(array(
						'cmd' => 'rm',
						'arg' => array(TEMPPATH . '/' . $this->name),
					));
					return false;
				},
				's' => function($cmd, &$data) use($lslist) {
					if(isset($lslist[$data])) {
						echo 'This filename already exists' . PHP_EOL;
						return true;
					} else {
						$this->name = $data;
						$data = NULL;
						return false;
					}
				},
			),
		));
		switch($cmd) {
			case 'n': case 'r': return false;
			case 's': return true;
		}
		// control should never reach here
		var_dump($cmd);
	}
	public function add(array $paras = array()) {
	// fill in data
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'noedittitle' =>
					'Whether to skip editing the title of the newly added entry',
			),
			'default' => array(
				'noedittitle' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		/*
		 * start adding data
		 */
		$this->setCurrentDate();
		if($this->isredirect()) {
			return true;
		}
		switch($this->getNameParser()->extension()) {
			case 'PDF': echo "Warning: uppercase file extension." . PHP_EOL;
			case 'pdf':
				$this->putpdfcontent();
				/*
				 * Try various options
				 */
				$successful = ($this->tryjstor() or
					$this->trybioone() or
					$this->trydoi() or
					$this->trygeodiv() or
					$this->trype() or
					$this->trygoogle() or
					$this->doiamnhinput() or
					$this->trymanual());
				break;
			default:
				// can't do tryjstor() etcetera when we're not in a PDF
				$successful = ($this->doiamnhinput() or
					$this->trymanual());
				break;
		}
		if(!$paras['noedittitle']) {
			$this->edittitle();
		}
		return $successful;
	}
	public function adddata() {
		if($this->isor('redirect', 'supplement') or $this->triedadddata) {
			return true;
		}
		if(!$this->needsdata()) {
			if(!$this->triedfinddoi and !$this->doi) {
				$this->finddoi();
			}
			return true;
		}
		// apply global settings
		$this->p->addmanual = false;
		$this->adddata_specifics();
		$this->triedadddata = true;
		// if there already is a DOI, little chance that we found something
		if($this->doi() !== false) {
			if(!$this->year or !$this->volume or !$this->start_page) {
				$this->expanddoi(array('verbose' => true));
				$this->needSave();
			}
			return true;
		} elseif($this->hasid()) {
			if(!$this->triedfinddoi) {
				$this->finddoi();
			}
			return true;
		}
		echo "Adding data for file $this->name... ";
		$tmp = new self(
			//TODO: figure out how to do this in the Sql version
			array($this->name, $this->folder, $this->sfolder, $this->ssfolder), 
			'l'
		);
		if(!$tmp->add(array('noedittitle' => true))) {
			echo "nothing found" . PHP_EOL;
		} else {
			foreach($tmp as $key => $value) {
				if($value && !$this->$key) {
					$this->$key = $value;
				}
			}
			$this->log('Added data');
			$this->needSave();
			echo "data added" . PHP_EOL;
		}
		if($this->hasid()) {
			return true;
		}
		if(!$this->triedfinddoi) {
			$this->finddoi();
		}
		if(!$this->triedfindurl) {
			$this->findurl();
		}
		return true;
	}
	private function findurl() {
		echo 'Trying to find a URL for file ' . $this->name . '... ';
		$json = self::fetchgoogle($this->googletitle());
		if($json === false) {
			throw new StopException;
		}
		// Google behaved correctly, but found nothing
		if($json === NULL) {
			echo 'nothing found' . PHP_EOL;
			return true;
		}
		$first = true;
		foreach($json->items as $result) {
			if($first) {
				echo PHP_EOL . $this->title . PHP_EOL;
				$this->echocite(array('mode' => 'paper'));
				echo PHP_EOL;
				$first = false;
			}
			echo "Title: " . $result->title . PHP_EOL;
			echo "URL: " . $result->link . PHP_EOL;
			while(true) {
				$cmd = $this->menu(array(
					'options' => array(
						'y' => 'this URL is correct',
						'n' => 'this URL is not correct',
						'q' => 'quit the function',
						'o' => 'open the URL',
						'r' => 'stop adding data',
						'u' => 'add a different URL',
						'i' => 'give information about this file',
						'e' => 'edit this file',
					),
					'prompt' => '> ',
				));
				switch($cmd) {
					case 'y':
						$this->needSave();
						$this->set(array('url' => $result->link));
						echo 'data added' . PHP_EOL;
						return true;
					case 'q':
						echo 'nothing found' . PHP_EOL;
						$this->triedfindurl = true;
						return false;
					case 'o':
						$this->shell(array('open', array($result->link)));
						break;
					case 'i':
						$this->inform();
						break;
					case 'n':
						break 2;
					case 'u':
						$this->set(array('url' => $this->getline('New url: ')));
						$this->needSave();
						echo 'data added' . PHP_EOL;
						return true;
					case 'r':
						echo 'nothing found' . PHP_EOL;
						throw new StopException;
					case 'e':
						$this->edit();
						break;
				}
			}
		}
		$this->triedfindurl = true;
	}
	private function finddoi() {
		if($this->doi() !== false) {
			return true;
		}
		if($this->journal() and $this->volume and $this->start_page) {
			echo "Trying to find DOI for file $this->name... ";
			$url = "http://www.crossref.org/openurl?pid=" . CROSSREFID 
				. "&title=" . urlencode($this->journal()) . "&volume=" 
				. urlencode($this->volume) . "&spage=" 
				. urlencode($this->start_page) . "&noredirect=true";
			$xml = @simplexml_load_file($url);
			// check success
			if($xml and $doi = $xml->query_result->body->query->doi) {
				$this->set(array('doi' => $doi));
				echo "data added" . PHP_EOL;
				$this->needSave();
				return true;
			} else {
				echo "nothing found" . PHP_EOL;
				$this->triedfinddoi = true;
				return false;
			}
		}
		return false;
	}
	public function findhdl() {
		if(!$this->isamnh() || ($this->hdl() !== false)) {
			return true;
		}
		echo 'Finding HDL for file ' . $this->name .
			' (' . $this->journal() . ' ' . $this->volume . ')' . PHP_EOL;
		switch($this->journal()) {
			case 'American Museum Novitates':
				$journalhdl = '2246/9';
				break;
			case 'Bulletin of the American Museum of Natural History':
				$journalhdl = '2246/7';
				break;
			case 'Anthropological Papers of the American Museum of Natural History':
				$journalhdl = '2246/6';
				break;
		}
		// construct search URL
		$url = 'http://digitallibrary.amnh.org/dspace/handle/' .
			$journalhdl .
			'/simple-search?query=series:' .
			$this->volume;
		$html = @file_get_contents($url);
		if(!$html) {
			echo 'Could not retrieve data (file ' . $this->name . ')' . PHP_EOL;
			return false;
		} else {
			echo 'Retrieved data from AMNH' . PHP_EOL;
		}
		// check whether we got one or several results
		if(strpos($html, 'Search produced no results.') !== false) {
			echo 'Could not find paper at AMNH' . PHP_EOL;
			return true;
		}
		if(strpos($html, 'Now showing items 1-1 of 1') !== false) {
			preg_match('/cocoon:\/\/metadata\/handle\/2246\/(\d+)\/mets\.xml/', $html, $matches);
			$hdl = '2246/' . $matches[1];
		} else {
			$this->shell(array('open', array($url)));
			$hdl = $this->getline('HDL: ');
			if($hdl === 'q') {
				return false;
			}
			if(strpos($hdl, 'http') !== false) {
				preg_match('/2246\/\d+$/', $hdl, $matches);
				$hdl = $matches[0];
			}
		}
		$this->set(array('hdl' => $hdl));
		echo 'Added HDL: ' . $hdl . PHP_EOL;
		$this->needSave();
		return true;
	}
	private function adddata_specifics() {
	// kitchen sink function for various stuff in adddata() territory, in order to keep addata() clean and to avoid lots of limited-use functions
		if(($this->journal === 'Estudios Geológicos' or preg_match('/springerlink|linkinghub\.elsevier|biomedcentral|ingentaconnect/', $this->url())) and !$this->doi()) {
			echo 'File ' . $this->name . ' has no DOI, but its data suggest it should have one.' . PHP_EOL;
			if(!$this->openurl()) {
				$this->searchgoogletitle();
			}
			$this->echocite();
			$doi = $this->getline('DOI: ');
			if($doi === 'q') {
				return false;
			} elseif($doi) {
				$this->set(array('doi' => $doi));
			}
		}
	}
	// Get the PDF text of a file and process it
	private function putpdfcontent() {
		// only do actual PDF files
		if(!$this->ispdf() or $this->isredirect()) {
			return false;
		}
		// only get first page
		$this->pdfcontent = trim(utf8_encode($this->shell(array(
			'cmd' => PDFTOTEXT,
			'arg' => array($this->path(), '-', '-l', '1'),
			'stderr' => BPATH . '/Catalog/data/pdftotextlog',
			'append-err' => true,
			'return' => 'output',
			'printout' => false,
			'exceptiononerror' => false,
		))));
		return ($this->pdfcontent === '') ? true : false;
	}
	public function getpdfcontent($paras = array()) {
		if(!$this->ispdf() or $this->isredirect()) {
			return false;
		}
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'force' => 'Whether to force the generation of a new pdfcontent',
			),
			'default' => array('force' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(!$paras['force']) {
			$this->p->getpdfcontentcache();
			if(isset($this->p->pdfcontentcache[$this->name])) {
				$this->pdfcontent =& $this->p->pdfcontentcache[$this->name];
				return $this->pdfcontent;
			}
			if(!$this->pdfcontent) {
				$this->putpdfcontent();
			}
		}
		else {
			$this->putpdfcontent();
		}
		$this->p->pdfcontentcache[$this->name] =& $this->pdfcontent;
		if(!$this->pdfcontent) {
			return false;
		}
		return $this->pdfcontent;
	}
	public function echopdfcontent() {
		$c = $this->getpdfcontent();
		if($c === false)
			return false;
		echo $c;
		return true;
	}
	public function findtitle_pdfcontent() {
	// tries to detect the title from $this->pdfcontent
	// assumes it is the first line with real running text
		$lines = explode("\n", $this->getpdfcontent());
		foreach($lines as $line) {
			// empty line
			if(!$line) continue;
			$line = trim($line);
			// titles are generally more than two words
			if(preg_match_all('/ /', $line, $matches) <= 2) continue;
			// probably not a title
			if(preg_match('/[©@·]/u', $line)) continue;
			// this looks like a volume-issue-pages kind of thing, or a date. This regex is matching a substantial number of existing articles; check how much of it is needed.
			if(preg_match('/\(\s*\d+\s*\)\s*:|\d{5}|\d\s*,\s*\d|\d+\s*\(\d+\)\s*\d+|\d+\s*\(\s*\d+\s*\),|\d+\s*\.\s*\d+\s*\.\s*\d+|Volume \d+|\s*(January|February|April|June|July|August|September|October|November|December)\s*\d+|\/\d|\d:\s*\d|\d\(\d+\)|\d\s*\(\d|\d\s*\)\s*:\s*\d|;\s*\d|\s+\/\s+|\d\s+\d|doi:|Vol\.?\s*\d+\s*No\.?\s*\d+|Pages?\s*\d|Vol\.\s*\d+,|\*{2}|\d\s+&|,\s*pp\.\s*\d|\d\)\s*\d/ui', $line)) continue;
			// no URLs in title
			if((strpos($line, 'http://') !== false) or (strpos($line, 'www.') !== false)) continue;
			// if there is no four-letter word in it, it's probably not a title
			if(!preg_match('/[a-zA-Z]{4}/', $line)) continue;
			// title will contain at least one uppercase letter
			if(!preg_match('/[A-Z]/', $line)) continue;
			// looks like an author listing (but this matches 24 real articles)
			if(preg_match('/Jr\.|\s([a-z]\s){2,}/u', $line)) continue;
			// [A-Z]\.\s*[A-Z]\.|
			// JSTOR, ScienceDirect stuff, probable author line, publisher line, other stuff
			if(preg_match('/collaborating with JSTOR|ScienceDirect|^By|^Issued|^Geobios|^Palaeogeography|^Published|^Printed|^Received|^Mitt\.|,$|^Journal compilation|^E \d+|^Zeitschrift|^J Mol|^Open access|^YMPEV|x{3}|^Reproduced|^BioOne|^Alcheringa|^MOLECULAR PHYLOGENETICS AND EVOLUTION|Ann\. Naturhist(or)?\. Mus\. Wien|Letter to the Editor|Proc\. Natl\. Acad\. Sci\. USA|American Society of Mammalogists|CONTRIBUTIONS FROM THE MUSEUM OF PALEONTOLOGY|American College of Veterinary Pathologists|Stuttgarter Beiträge zur Naturkunde|^The Newsletter|^Short notes|^No\. of pages|Verlag|^This copy|Southwestern Association of Naturalists|^Peabody Museum|^(c) |^Number|^Occasional Papers|^Article in press|^Museum|^The university|^University|^Journal|^Key words|^International journal|^Terms? of use|^Bulletin|^A journal|^The Bulletin|^Academy|en prensa|^American Journal|^Contributions from|Museum of Natural History$|^The American|^Notes on geographic distribution$|Publications$|Sistema de Información Científica|Press$|^Downloaded|^Serie|issn|^Society|University of|Elsevier|^Australian Journal|BIOLOGICAL SOCIETY OF WASHINGTON/ui', $line)) continue;
			// if it starts with a year, it's unlikely to be a title
			if(preg_match('/^\d{3}/u', $line)) continue;
			// if we got to such a long line, we're probably already in the abstract and we're not going to find the title. Longest title in database as of November 24, 2011 is 292
			if(strlen($line) > 300) return false;
			// how is this?
			else return $line;
		}
		return false;
	}
	private function findtitle_specific() {
	// find title in specific journals
		// this is now barely useful anymore
		$j = array();
		$has = function($in) use(&$j) {
			return isset($j[$in]) && ($j[$in] === true);
		};
		if(
			$j['mammalia'] = preg_match("/(Mammalia|MAMMALIA), (t|I)\. /", $this->pdfcontent) or
			// this is also likely Mammalia ("par" on 2nd line)
			$j['mammalia'] = preg_match("/^[^\n]*\n+(par|PAR)\n+/", $this->pdfcontent) or
			// this too
			$j['mammalia'] = preg_match("/^[^\n]*\n+(by|BY)\n+/", $this->pdfcontent) or
			// and this
			$j['mammalia'] = preg_match("/MAMMALIA · /", $this->pdfcontent) or
			$j['jvp'] = preg_match("/^\s*Journal of Vertebrate Paleontology/", $this->pdfcontent) or
			$j['jparas'] = preg_match("/^\s*J. Parasitol.,/", $this->pdfcontent) or
			$j['bioljlinnsoc'] = preg_match("/^\s*Biological Journal of the Linnean Society, /", $this->pdfcontent) or
			$bioljlinnsoc2 = preg_match("/^[^\n]+\n\nBiological Journal of the Linnean Society, /", $this->pdfcontent) or
			$j['zooljlinnsoc'] = preg_match("/^\s*Zoological Journal of the Linnean Society, /", $this->pdfcontent) or
			$j['swnat'] = preg_match("/^\s*THE SOUTHWESTERN NATURALIST/", $this->pdfcontent) or
			$j['wnanat'] = preg_match("/^\s*Western North American Naturalist/", $this->pdfcontent) or
			$j['mammreview'] = preg_match("/^\s*Mammal Rev. /", $this->pdfcontent) or
			$j['mammstudy'] = preg_match("/^\s*Mammal Study /", $this->pdfcontent) or
			$j['jpaleont'] = preg_match("/^\s*(Journal of Paleontology|J. Paleont.)/", $this->pdfcontent) or
			$j['jbiogeogr'] = preg_match("/^s*(Journal of Biogeography)/", $this->pdfcontent) or
			$j['amjprim'] = preg_match("/^\s*American Journal of Primatology/", $this->pdfcontent) or
			$j['ajpa'] = preg_match("/^\s*AMERICAN JOURNAL OF PHYSICAL ANTHROPOLOGY/", $this->pdfcontent) or
			$j['oryx'] = preg_match("/^\s*Oryx /", $this->pdfcontent) or
			$j['jmamm'] = preg_match("/^\s*Journal of Mammalogy,/", $this->pdfcontent)
			) {
			echo "Found BioOne/Mammalia/Wiley paper; searching Google to find a DOI." . PHP_EOL;
			/*
			 * find title
			 */
			// title ought to be first line
			if($has('mammalia')) {
				$title = preg_replace("/^([^\n]+).*/su", "$1", $this->pdfcontent);
			} else if($has('jvp')) {
				$title = preg_replace("/^[^\n]+\n+((ARTICLE|SHORT COMMUNICATION|RAPID COMMUNICATION|NOTE|FEATURED ARTICLE|CORRECTION|REVIEW)\n+)?([^\n]+)\n.*/su", "$3", $this->pdfcontent);
			} else if($has('jparas')) {
				$title = preg_replace("/.*American Society of Parasitologists \d+\s*([^\n]+)\n.*/su", "$1", $this->pdfcontent);
			} else if($has('swnat')) {
				$title = preg_replace("/^[^\n]+\n+[^\n]+\n+([^\n]+).*/su", "$1", $this->pdfcontent);
			// title is after a line of text plus two newlines; works often
			} else if($has('wnanat') || $has('mammreview') || $has('jpaleont') || $has('jbiogeogr') || $has('ajpa') || $has('oryx') || $has('jmamm') || $has('bioljlinnsoc') || $has('mammstudy') || $has('zooljlinnsoc')) { 
				$title = preg_replace("/^[^\n]+\n+((ORIGINAL ARTICLE|SHORT COMMUNICATION|Short communication|Blackwell Science, Ltd|CORRECTION|REVIEW)\n+)?([^\n]+).*/su", "$3", $this->pdfcontent);
			// this needs some other possibilities
			} else if($has('amjprim')) {
				$title = preg_replace("/^[^\n]+\n\n((BRIEF REPORT|RESEARCH ARTICLES)\s)?([^\n]+).*/su", "$3", $this->pdfcontent);
			} else if($has('bioljlinnsoc2')) {
				$title = preg_replace("/^[^\n]+\n\n[^\n]+\n\n([^\n]+).*/su", "$1", $this->pdfcontent);
			}
			if(!isset($title) || preg_match("/\n/", trim($title))) {
				echo "Error: could not find title." . PHP_EOL;
				return false;
			}
			if($has('jvp') || $has('jparas') || $has('wnanat') || $has('swnat') || $has('mammstudy') || $has('jpaleont') || $has('jmamm')) {
				$title .= " site:bioone.org";
			} else if($has('mammalia')) {
				$title .= " site:reference-global.com";
			} else if($has('bioljlinnsoc') || $has('bioljlinnsoc') || $has('mammreview') || $has('jbiogeogr') || $has('ajpa') || $has('zooljlinnsoc')) {
				$title .= " site:wiley.com";
			}
			return $title;
		}
		return false;
	}
	public function test_pdfcontent() {
	// test whether the PDFcontent functions are producing correct results
	// also useful for detecting damaged PDF files
		echo 'File ' . $this->name . PHP_EOL;
		if($this->putpdfcontent())
			echo $this->findtitle_pdfcontent() . PHP_EOL;
		return true;
	}
	// Try to determine the citation from the PDF content
	private function tryjstor() {
		if(!preg_match("/(Stable URL: http:\/\/www\.jstor\.org\/stable\/| Accessed: )/", $this->pdfcontent))
			return false;
		echo "Detected JSTOR file; extracting data." . PHP_EOL;
		$splittext = preg_split("/\nYour use of the JSTOR archive indicates your acceptance of JSTOR's Terms and Conditions of Use/", $this->pdfcontent);
		$head = $splittext[0];
		// get rid of occasional text above relevant info
		$head = preg_replace("/^.*\n\n/", "", $head);
		// bail out
		if(preg_match('/Review by:/', $head)) {
			echo 'Unable to process data' . PHP_EOL;
			return false;
		}
		
		// split into fields
		$head = preg_split("/( Author\(s\): |\s*(Reviewed work\(s\):.*)?Source: | Published by: | Stable URL: |( \.)? Accessed: )/", $head);
		
		$data = array();
		// handle the easy ones
		$data['title'] = $head[0];
		$data['doi'] = '10.2307/' . substr($head[4], 28);
		// problem sometimes
		if(!preg_match("/(, Vol\. |, No\. | \(|\), pp?\. )/", $head[2])) {
			echo 'Unable to process data' . PHP_EOL;
			return false;
		}
		/*
		 * Process "source" field
		 */
		$source = preg_split("/(, Vol\. |, No\. | \(|\), pp?\. )/", $head[2]);
		$data['journal'] = $source[0];
		$data['volume'] = $source[1];
		// issue may have been omitted
		$data['issue'] = isset($source[4]) ? $source[2] : NULL;
		// year
		$year = isset($source[4]) ? $source[3] : $source[2];
		$data['year'] = preg_replace("/^.*, /", "", $year);
		// start and end pages
		$pages = isset($source[4]) ? $source[4] : $source[3];
		$pages = explode('-', $pages);
		$data['start_page'] = $pages[0];
		$data['end_page'] = isset($pages[1]) ? $pages[1] : $pages[0];
		/*
		 * Process "authors" field
		 * Will fail with various variants, including double surnames
		 */
		$authors = $head[1];
		$authors = preg_split("/(, | and )/", $authors);
		// array for correctly formatted authors
		$fmtauth = array();
		foreach($authors as $author) {
			$author = preg_split("/\s/", $author);
			$lastname = array_pop($author) . ", ";
			foreach($author as $firstname) {
				$firstname = preg_replace("/(?<=^[A-Z]).*$/", ".", $firstname);
				$lastname .= $firstname;
			}
			$fmtauth[] = $lastname;
		}
		$authorstring = '';
		foreach($fmtauth as $key => $author) {
			if($key !== 0)
				$authorstring .= "; ";
			$authorstring .= $author;
		}
		$data['authors'] = $authorstring;
		// if it isn't, this code fails miserably anyway
		$data['type'] = self::JOURNAL;
		$this->set($data);
		return true;
	}
	private function trybioone() {
		if(!preg_match("/BioOne \(www\.bioone\.org\) is an electronic aggregator of bioscience research content/", $this->pdfcontent))
			return false;
		echo "Detected BioOne file; extracting data." . PHP_EOL;
		$pdftext = preg_split("/\n\nBioOne \(www\.bioone\.org\) is an electronic aggregator of bioscience research content/", $this->pdfcontent);
		$head = trim($pdftext[0]);
		// get rid of occasional text above relevant info
		$head = preg_replace("/^.*\n\n/", "", $head);
		// split into fields
		$head = preg_split("/(\sAuthor\(s\): | Source: |\. Published By: | URL: )/", $head);
		
		$data = array();
		// handle the easy ones
		$data['title'] = $head[0];
		$data['doi'] = preg_replace("/^http:\/\/www\.bioone\.org\/doi\/full\//", '', $head[4]);
		/*
		 * Process "source" field
		 */
		$source = $head[2];
		$source = preg_split("/(, |\(|\)?:|\. )/", $source);
		$data['journal'] = $source[0];
		$data['volume'] = str_replace('Number ', '', $source[1]);
		// issue may have been omitted
		$data['issue'] = $source[4] ? $source[2] : NULL;
		// year
		$year = $source[4] ? $source[4] : $source[3];
		$data['year'] = preg_replace("/[\.\s]/", "", $year);
		// start and end pages
		$pages = $source[4] ? $source[3] : $source[2];
		$pages = preg_split("/-/", $pages);
		$data['start_page'] = $pages[0];
		$data['end_page'] = $pages[1] ? $pages[1] : $pages[0];
		/*
		 * Process "authors" field
		 * Will fail with various variants, including double surnames
		 */
		$authors = $head[1];
		$authors = preg_split("/(,? and |, (?!and)| & )/", $authors);
		// array for correctly formatted authors
		$fmtauth = array();
		foreach($authors as $author) {
			$author = preg_split("/\s/", $author);
			$lastname = array_pop($author) . ", ";
			foreach($author as $firstname) {
				$firstname = preg_replace("/(?<=^[A-Z]).*$/", ".", $firstname);
				$lastname .= $firstname;
			}
			$fmtauth[] = $lastname;
		}
		foreach($fmtauth as $key => $author) {
			if($key !== 0)
				$authorstring .= "; ";
			$authorstring .= $author;
		}
		$data['authors'] = $authorstring;
		// if it isn't, this code fails miserably anyway
		$data['type'] = self::JOURNAL;
		$this->set($data);
		return true;
	}
	private function trygeodiv() {
		if(!preg_match("/GEODIVERSITAS · /", $this->pdfcontent))
			return false;
		echo "Detected Geodiversitas paper." . PHP_EOL;
		preg_match("/\n\n([^\n]*)\n\nKEY/s", $this->pdfcontent, $cite);
		if(!$cite = trim($cite[1]))
			return false;
		// split into fields
		$head = preg_split("/((?<=\.) (?=\d{4})|\. -- |\. Geodiversitas |(?<=\d) \((?=\d)|(?<=\d)\) : (?=\d))/", $cite);
		
		$data = array();
		// handle the easy ones
		$data['journal'] = "Geodiversitas";
		$data['title'] = $head[2];
		$data['year'] = $head[1];
		$data['volume'] = $head[3];
		$data['issue'] = $head[4];
		/*
		 * Process pages
		 */
		$pages = str_replace(".", "", $head[5]);
		$pages = explode("-", $pages);
		$data['start_page'] = $pages[0];
		$data['end_page'] = preg_replace("/\n.*$/", "", $pages[1]);
		/*
		 * Process "authors" field
		 */
		// kill extraneous stuff
		$authors = preg_replace("/.*\n([^\n]+)$/", "$1", $head[0]);
		// kill spaces between initials
		$authors = str_replace(". ", ".", $authors);
		// semicolons between authors
		$authors = preg_replace("/((?<=\.), |& )/", "; ", $authors);
		// comma before initials
		$data['authors'] = preg_replace("/(?<=[a-z]) (?=[A-Z]\.)/", ", ", $authors);
		$data['type'] = self::JOURNAL;
		$this->set($data);
		return true;
	}
	private function trype() {
	// Palaeontologia Electronica
		// PE has a citation block after either a new line or something like "Accepted: 5 October 2011"
		if(!preg_match("/(\n|Acceptance:\s*(\d+\s*[A-Z][a-z]+|\?\?)\s*\d+\s*)(?!PE Article Number)([^\n]+Palaeontologia Electronica Vol\.[^\n]+)(\s*\$|\n)/u", $this->pdfcontent, $matches))
			return false;
		$citation = $matches[3];
		$processauthors = function($in) {
			$authors = explode(', ', $in);
			$out = '';
			foreach($authors as $key => $aut) {
				if(($key % 2) === 0) { // even index
					$out .= preg_replace('/^and /u', '', $aut) . ', ';
				}
				else {
					$auta = explode(' ', $aut);
					foreach($auta as $autp) {
						$out .= $autp[0] . '.';
					}
					$out .= '; ';
				}
			}
			return preg_replace('/; $/u', '', $out);
		};
		$data = array();
		if(preg_match(
			"/^(.*?), (\d{4})\. (.*?), Palaeontologia Electronica Vol\. (\d+), Issue (\d+); ([\dA-Z]+):(\d+)p, [\dA-Za-z]+; ([^\s]*)\$/u",
			$citation,
			$matches
		)) {
			$data['authors'] = $processauthors($matches[1]);
			$data['year'] = $matches[2];
			$data['title'] = $matches[3];
			$data['journal'] = 'Palaeontologia Electronica';
			$data['volume'] = $matches[4];
			$data['issue'] = $matches[5];
			$data['start_page'] = $matches[6];
			$data['pages'] = $matches[7];
			$data['url'] = $matches[8];
		}
		else if(preg_match(
			"/^(.*?) (\d{4})\. (.*?). Palaeontologia Electronica Vol\. (\d+), Issue (\d+); ([\dA-Z]+):(\d+)p; ([^\s]*)\$/u",
			$citation,
			$matches
		)) {
			$data['authors'] = $processauthors($matches[1]);
			$data['year'] = $matches[2];
			$data['title'] = $matches[3];
			$data['journal'] = 'Palaeontologia Electronica';
			$data['volume'] = $matches[4];
			$data['issue'] = $matches[5];
			$data['start_page'] = $matches[6];
			$data['pages'] = $matches[7];
			$data['url'] = 'http://' . $matches[8];
		} else {
			return false;
		}
		$data['type'] = self::JOURNAL;
		$this->set($data);
		return true;
	}
	private function trydoi() {
		if(preg_match_all("/(doi|DOI)\s*(\/((full|abs|pdf)\/)?|:|\.org\/)?\s*(?!URL:)([^\s]*?),?\s/su", $this->pdfcontent, $matches)) {
			echo "Detected possible DOI." . PHP_EOL;
			foreach($matches[5] as $match) {
				$doi = Sanitizer::trimdoi($match);
				// PNAS tends to return this
				if(preg_match('/^10.\d{4}\/?$/', $doi))
					$doi = preg_replace("/.*?10\.(\d{4})\/? ([^\s]+).*/s", "10.$1/$2", $this->pdfcontent);
				// Elsevier accepted manuscripts
				if(in_array($doi, array("Reference:", "Accepted Manuscript"))) {
					if(preg_match("/Accepted date: [^\s]+ ([^\s]+)/s", $this->pdfcontent, $doi)) {
						$doi = $doi[1];
					} else {
						echo 'Could not find DOI' . PHP_EOL;
						return false;
					}
				}
				// get rid of false positive DOIs containing only letters or numbers, or containing line breaks
				if($doi && !preg_match("/^([a-z\(\)]*|\d*)$/", $doi) && !preg_match("/\n/", $doi)) {
					// remove final period
					$doi = preg_replace("/\.$/", "", $doi);
					$this->set(array('doi' => $doi));
					echo 'Found DOI: ' . $doi . PHP_EOL;
					return $this->expanddoi();
				}
				else {
					echo "Could not find DOI: $doi." . PHP_EOL;
				}
			}
		}
		return false;
	}
	private function trygoogle() {
		// find title
		($title = $this->findtitle_specific()) or
			($title = $this->findtitle_pdfcontent());
		if($title === false) {
			return false;
		}
		// show title so it's possible to confirm it's right
		echo "Title: $title" . PHP_EOL;
		/*
		 * get data
		 */
		// construct url
		$search = self::googletitle($title);
		// fetch data
		$cjson = self::fetchgoogle($search);
		if($cjson === false) {
			// we didn't find anything
			echo "Could not find any results in Google" . PHP_EOL;
			return false;
		}
		/*
		 * process
		 */
		foreach($cjson->items as $result) {
			echo "Title: " . $result->title . PHP_EOL;
			echo "URL: " . $result->link . PHP_EOL;
			$url = $result->link;
			$cmd = $this->menu(array(
				'options' => array(
					'y' => 'this URL is correct',
					'n' => 'this URL is not correct',
					'q' => 'quit the function',
					'o' => 'open the URL',
					'r' => 'stop adding data',
				),
				'process' => array(
					'o' => function() use(&$url) {
						$this->shell(array(
							'cmd' => 'open', 
							'arg' => array($url),
						));
						return true;
					},
					'q' => function(&$cmd) {
						$cmd = false;
						return false;
					},
				),
			));
			switch($cmd) {
				case 'y':
					$doi = preg_replace(array("/^.*\/doi\/(abs|pdf|full|url|pdfplusdirect)?\/?/", "/(\?.*|\/(abstract|full|pdf))$/"), array("", ""), $result->link);
					if($doi === $result->link) {
						$this->set(array('url' => $result->link));
						// return false, not true, because this doesn't mean we don't have to do manual input for other stuff
						return false;
					}
					else {
						$this->set(array('doi' => trim($doi)));
						return $this->expanddoi();
					}
				case 'n':
					break;
				case 'r':
					throw new StopException;
				default:
					return $cmd;
			}
		}
	}
	// Getting stuff from Google
	private function googletitle($title = '') {
		if($title === '') {
			$title = $this->title;
		}
		return urlencode(preg_replace("/\(|\)|;|-+\\/(?=\s)|(?<=\s)-+|<\/?i>/", "", $title));
	}
	private static function fetchgoogle($search) {
		// get data from Google
		$url = "https://www.googleapis.com/customsearch/v1?key=" . GOOGLEKEY . "&cx=" . GOOGLECUS . "&q=" . $search;
		$json = @file_get_contents($url);
		if(!$json) {
			echo "Error: nothing found in Google." . PHP_EOL;
			return false;
		}
		// decode into PHP-readable format
		$cjson = json_decode($json);
		if($cjson === NULL and $json !== 'null' and $json !== 'NULL') {
			echo "Error: could not decode Google results." . PHP_EOL;
			return false;
		}
		// this means we got good input, but found no results in Google
		if((!isset($cjson->items)) or (!is_array($cjson->items))) {
			echo "Could not find anything in Google." . PHP_EOL;
			return false;
		}
		return $cjson;
	}
	// Manual input
	private function doiamnhinput() {
		if(!$this->p->addmanual) {
			return false;
		}
		$this->echopdfcontent();
		$converter = $this->getConverterToType(array(
			'excluded' => array(self::REDIRECT),
		));
		$hdlValidator = self::getValidator('hdl');
		$doiValidator = self::getValidator('doi');
		return $this->menu(array(
			'head' => 
				'If this file has a DOI or AMNH handle, please enter it. Otherwise, enter the type of the file.',
			'helpinfo' =>
				'In addition to the regular commands, the following synonyms are accepted for the several types:' . PHP_EOL 
				. $this->getTypeSynonymsAsString(array(
					'excluded' => array(self::REDIRECT),
				)),
			'options' => array(
				'o' => 'open the file',
				'r' => 're-use a citation from a NOFILE entry',
				// fake commands:
				// 't' => 'set type',
				// 'd' => 'enter doi',
				// 'h' => 'enter hdl',
			),
			'processcommand' => function($cmd, &$data) use($converter, $doiValidator, $hdlValidator) {
				if($cmd === 'o' || $cmd === 'r') {
					return $cmd;
				}
				$type = $converter($cmd);
				if($type !== false) {
					$data = $type;
					return 't';
				}
				// try doi
				$cmd = Sanitizer::trimdoi($cmd);
				if($doiValidator($cmd)) {
					$data = $cmd;
					return 'd';
				}
				// try hdl
				if(substr($cmd, 0, 22) === 'http://hdl.handle.net/') {
					$cmd = substr($cmd, 22);
				}
				if($hdlValidator($cmd)) {
					$data = $cmd;
					return 'h';
				} else {
					return false;
				}
			},
			'validfunction' => function() {
				return true;
			},
			'process' => array(
				'o' => function() {
					$this->openf();
					return true;
				},
				'r' => function(&$cmd, &$data) {
					if($this->reuseNoFile()) {
						$cmd = true;
						$data = NULL;
						return false;
					} else {
						return true;
					}
				},
				't' => function(&$cmd, &$data) {
					$this->type = $data;
					$cmd = false;
					$data = NULL;
					return false;
				},
				'h' => function(&$cmd, &$data) {
					$this->set(array('hdl' => $data));
					if($this->expandamnh()) {
						$cmd = true;
						$data = NULL;
						return false;
					} else {
						echo "Could not find data at the AMNH." . PHP_EOL;
						return true;					
					}
				},
				'd' => function(&$cmd, &$data) {
					$this->set(array('doi' => $data));
					if($this->expanddoi()) {
						$cmd = true;
						$data = NULL;
						return false;
					} else {
						return true;
					}
				},
			),
		));
	}
	private function reuseNofile() {
		$handle = $this->menu(array(
			'head' => 'Enter the citation handle:',
			'options' => array('q' => 'Stop trying csvrefs'),
			'validfunction' => function($in) {
				return $this->p->has($in);
			}
		));
		if($handle === 'q') {
			return false;
		}
		$blacklist = array('addmonth', 'addday', 'addyear', 'adddate', 'name', 'folder', 'sfolder', 'ssfolder');
		$data = array();
		foreach($this->p->get($handle) as $key => $value) {
			if(!in_array($key, $blacklist, true)) {
				$data[$key] = $value;
			}
		}
		$this->set($data);
		// make redirect
		$this->p->addRedirect(array($handle, $this->name));
		echo 'Data copied.' . PHP_EOL;
		if($this->ynmenu('Do you want to review and edit information now associated with this file?')) {
			$this->inform();
			$this->edit();
		}
		return true;
	}
	protected function trymanual() {
		// apply global setting
		if(!$this->p->addmanual) {
			return false;
		}
		switch($this->type) {
			case self::JOURNAL:
				$fields = array(
					'authors', 'year', 'title', 'journal', 'volume', 'issue', 
					'start_page', 'end_page', 'url',
				);
				break;
			case self::CHAPTER:
				$fields = array(
					'authors', 'year', 'title', 'start_page', 'end_page',
					'parent', 'url',
				);
				break;
			case self::BOOK:
				$fields = array(
					'authors', 'year', 'title', 'pages', 'publisher', 'isbn',
				);
				break;
			case self::THESIS:
				$fields = array(
					'authors', 'year', 'title', 'pages', 'publisher', 'series',
				);
				break;
			case self::SUPPLEMENT:
				$fields = array(
					'title', 'parent',
				);
				break;
			case self::WEB:
			case self::MISCELLANEOUS:
				$fields = array(
					'authors', 'year', 'title', 'url',
				);
		}
		foreach($fields as $field) {
			if(!$this->manuallyFillProperty($field)) {
				return true;
			}
		}
		return true;
	}
	protected function fillScalarProperty($field) {
		// returns false if we want to stop inputting data, true if we need more
		$processor = self::getFieldObject($field)->getProcessor();
		$validator = self::getFieldObject($field)->getValidator();
		return $this->menu(array(
			'prompt' => $field . ': ',
			'options' => array(
				'e' => "Enter the file's command-line interface",
				'd' => "Try entering a DOI again",
				's' => "Save the file now",
				'o' => "Open the file",
				// Fake:
				// 'set' => "Set an option",
			),
			'processcommand' => function($in, &$data) use($processor) {
				// enable having 'e' or 's' as field by typing '\e'
				if(isset($in[0]) and ($in[0] === '\\')) {
					$in = substr($in, 1);
				}
				$data = $processor($in);
				return 'set';
			},
			'validfunction' => function($cmd, $options, $data) use($validator) {
				if($cmd === 'set') {
					return $validator($data);
				} else {
					return array_key_exists($cmd, $options);
				}
			},
			'process' => array(
				'e' => function() {
					$this->edit();
					return true;
				},
				'o' => function() {
					$this->openf();
					return true;
				},
				'd' => function() {
					$this->doiamnhinput();
					return true;
				},
				's' => function(&$cmd) {
					$cmd = false;
					return false;
				},
				'set' => function(&$cmd, $data) use($field) {
					$cmd = true;
					$this->set(array($field => $data));
					return false;
				},
			),
		));
	}
	abstract protected function manuallyFillProperty();
	// Expanding AMNH data and DOIs
	private function expandamnh(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array('text' => 'Text of HTML file to be parsed'),
			'default' => array('text' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$hdl = $this->hdl();
		if($hdl === false) {
			return false;
		}
		// load document. Suppress errors because it's not our fault if the AMNH's HTML is messed up.
		libxml_use_internal_errors(true);
		$doc = new DOMDocument();
		if($paras['text'] !== false) {
			try {
				$doc->loadHTML($paras['text']);
			} catch(Exception $e) {
				echo 'Unable to load data from AMNH' . PHP_EOL;
				return false;				
			}
		} else {
			try {
				$doc->loadHTMLFile(
					'http://digitallibrary.amnh.org/dspace/handle/' .
					$hdl . '?show=full');
			} catch(Exception $e) {
				echo 'Unable to load data from AMNH' . PHP_EOL;
				return false;
			}
		}
		libxml_use_internal_errors(false);
		$list = $doc->getElementsByTagName('tr');
		$authors = '';
		$data = array();
		for($i = 0; $i < $list->length; $i++) {
			$row = $list->item($i);
			// only handle actual data
			if(strpos($row->attributes->getNamedItem('class')->nodeValue, 'ds-table-row') !== 0)
				continue;
			$label = $row->childNodes->item(0)->nodeValue;
			$value = $row->childNodes->item(2)->nodeValue;
			switch($label) {
				case 'dc.contributor.author':
					// remove year of birth
					$value = preg_replace('/, [\d\-]+| \(.*\)/u', '', $value);
					$authors .= preg_replace(
						'/(?<=, |\.)([A-Z])\w*\s*/u',
						'$1.',
						$value
					) . '; ';
					break;
				case 'dc.date.issued':
					$data['year'] = $value;
					break;
				case 'dc.description':
					if(preg_match('/^p\.\s*\[?(\d+)\]?-\[?(\d+)\]?/u', $value, $matches)) {
						$data['start_page'] = $matches[1];
						$data['end_page'] = $matches[2];
					} elseif(preg_match('/^p\.\s*\[?(\d+)\]?/u', $value, $matches)) {
						$data['start_page'] = $data['end_page'] = $matches[1];
					} elseif(preg_match("/^(\d+)\s*p\./u", $value, $matches)) {
						$data['start_page'] = 1;
						$data['end_page'] = $matches[1];
					}
					break;
				case 'dc.relation.ispartofseries':
					$series = preg_split('/;\s+(no|vol|v)\.\s+|, article /u', $value);
					$data['journal'] = trim($series[0]);
					$data['volume'] = trim($series[1]);
					if(isset($series[2])) {
						$data['issue'] = trim(str_replace('.', '', $series[2]));
					}
					break;
				case 'dc.title': // title, with some extraneous stuff
					$data['title'] = trim(preg_replace(
						'/\. (American Museum novitates|Bulletin of the AMNH|Anthropological papers of the AMNH|Memoirs of the AMNH|Bulletin of the American Museum of Natural History).*$/u',
						'',
						$value
					));
					break;
			}
		}
		// final cleanup
		$data['authors'] = trim(preg_replace(
			array('/\.+\s*/u', '/; $/u'),
			array('.', ''),
			$authors
		));
		$data['type'] = self::JOURNAL;
		$this->set($data);
		return true;
	}
	protected function expanddoi($paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'overwrite' => 'Whether to overwrite existing data',
				'verbose' => 'Whether to mention data that differs from existing data',
			),
			'default' => array(
				'overwrite' => false,
				'verbose' => false
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$doi = $this->doi();
		// fetch data
		$url = "http://www.crossref.org/openurl/?pid=" . CROSSREFID . "&id=doi:" . $doi . "&noredirect=true";
		$xml = @simplexml_load_file($url);
		// check success
		if($xml &&
			$xml->query_result && $xml->query_result->body->query &&
			($arr = $xml->query_result->body->query->attributes()) &&
			(string)$arr === "resolved") {
			echo 'Retrieved data for DOI ' . $doi . PHP_EOL;
			$result = $xml->query_result->body->query;
		} else {
			echo 'Could not retrieve data for DOI ' . $doi . PHP_EOL;
			return false;
		}
		/*
		 * process data
		 */
		// variables we process from the API result
		$vars = array('volume', 'issue', 'start_page', 'end_page', 'year', 'title', 'journal', 'isbn', 'authors');
		$data = array();
		
		$doiType = (string) $result->doi->attributes()->type;
		// values from http://www.crossref.org/schema/queryResultSchema/crossref_query_output2.0.xsd
		switch($doiType) {
			case 'journal_title':
			case 'journal_issue':
			case 'journal_volume':
			case 'journal_article':
				$data['type'] = self::JOURNAL;
				break;
			case 'conference_paper':
			case 'component':
			case 'book_content':
				$data['type'] = self::CHAPTER;
				break;
			case 'dissertation':
				$data['type'] = self::THESIS;
				break;
			case 'conference_title':
			case 'conference_series':
			case 'book_title':
			case 'book_series':
				$data['type'] = self::BOOK;
				break;
			case 'report-paper_title':
			case 'report-paper_series':
			case 'report-paper_content':
			case 'standard_title':
			case 'standard_series':
			case 'standard_content':
			default:
				$data['type'] = self::MISCELLANEOUS;
				break;
		}
		// kill leading zeroes
		$volume = preg_replace("/^0/u", "", (string)$result->volume);
		$issue = preg_replace("/^0/u", "", (string)$result->issue);
		$start_page = preg_replace("/^0/u", "", (string)$result->first_page);
		$end_page = preg_replace("/^0/u", "", (string)$result->last_page);
		$year = (string)$result->year;
		$title = (string)$result->article_title;
		$journal = (string)$result->journal_title;
		$booktitle = (string)$result->volume_title;
		$isbn = (string)$result->isbn;
		$authorsraw = $result->contributors->children();
		$authors = '';
		if($result->contributors->count()) foreach($authorsraw as $author) {
			if((string)$author->attributes() !== "first")
				$authors .= "; ";
			$authors .= ucwords(mb_strtolower((string)$author->surname, 'UTF-8'));
			$authors .= ", ";
			$authors .= preg_replace(array("/([^\s])[^\s]*/u", "/\s/u"), array("$1.", ""), (string)$author->given_name);
		}
		foreach($vars as $var) {
			if(${$var}) {
				// echo differences if verbose is set
				if($paras['verbose']) {
					// loose comparison is intentional here
					if(simplify(${$var}) != simplify($this->$var)) {
						echo 'Different data from expanddoi(). File ' . $this->name . '; var ' . $var . PHP_EOL;
						echo 'Existing data: ' . $this->$var . PHP_EOL;
						echo 'New data: ' . ${$var} . PHP_EOL;
					}
				}
				// overwrite everything if overwrite is set; else only if no existing data
				if($paras['overwrite']) {
					$data[$var] = ${$var};
				} elseif(!$this->$var) {
					$data[$var] = ${$var};
				}
			}
		}
		if($booktitle !== '') {
			if(strpos($booktitle, '/') === false and strpos($this->doi, 'bhl') !== false) {
				$this->fillEnclosingFromTitle($booktitle);
			} elseif(!$this->title) {
				$data['title'] = $booktitle;
			}
		}
		$this->set($data);
		return true;
	}
	/* 
	 * Processing PDFs
	 */
	public function burst() {
	// bursts a PDF file into several files
		echo 'Bursting file "' . $this->name . '". Opening file.' . PHP_EOL;
		$this->shell(array('open', array(BURSTPATH . '/' . $this->name)));
		
		$this->menu(array(
			'head' => 'Enter file names and page ranges',
			'prompt' => 'File name: ',
			'options' => array(
				'c' => 'continue with the next file',
				'q' => 'quit',
				// fake commands, used internally by processcommand/process
				// a => add this file
				// i => ignore
			),
			'processcommand' => function($cmd, &$data) {
				if($cmd === 'c' || $cmd === 'q') {
					return $cmd;
				} else {
					$data = $cmd;
					if($this->p->has($cmd)) {
						if(!$this->ynmenu('A file with this name already exists. Do you want to continue anyway?')) {
							return 'i';
						}
					}
					return 'a';
				}
			},
			'validfunction' => function() {
				return true;
			},
			'process' => array(
				'q' => function() { 
					throw new StopException('burst'); 
				},
				'i' => function() {
					return true;
				},
				'c' => function() {
					$this->shell(array(
						'cmd' => 'mv',
						'arg' => array('-n',
							BURSTPATH . '/' . $this->name,
							BURSTPATH . '/Old/' . $this->name)
					));
					return false;				
				},
				'a' => function($cmd, $data) {
					$range = $this->menu(array(
						'prompt' => 'Page range: ',
						'validfunction' => function($range) {
							return preg_match('/^\d+-\d+$/', $range);
						},
					));
					$srange = explode('-', $range);
					$from = $srange[0];
					$to = $srange[1];
					$this->shell(array(
						'cmd' => 'gs',
						'arg' => array(
							'-dBATCH', '-dNOPAUSE', '-q', '-sDEVICE=pdfwrite',
							'-dFirstPage=' . $from, '-dLastPage=' . $to,
							'-sOUTPUTFILE=' . TEMPPATH . '/' . $data,
							BURSTPATH . '/' . $this->name,
						),
					));
					echo 'Split off file ' . $data . PHP_EOL;
					$file = new self(NULL, 'e', $this->p);
					$file->name = $data;
					$this->p->addNewFile($file);
					return true;
				},
			),
		));
		return true;
	}
	public function removefirstpage() {
		$tmpPath = $this->path(array('folder' => true, 'type' => 'none')) 
			. '/tmp.pdf';
		$path = $this->path(array('type' => 'none'));
		$this->shell(array(
			'cmd' => 'gs',
			'arg' => array(
				'-dBATCH', '-dNOPAUSE', '-q', '-sDEVICE=pdfwrite', 
				'-dFirstPage=2', '-sOUTPUTFILE=' . $tmpPath, $path
			),
		));
		// open files for review
		$this->shell(array('open', array($tmpPath)));
		$this->openf();
		if($this->ynmenu("Do you want to replace the file?")) {
			$this->shell(array('mv', array($tmpPath, $path)));
			return true;
		} else {
			$this->shell(array('rm', array($tmpPath)));
			return false;
		}
	}
	/*
	 * Other utilities
	 */
	public function getkey() {
		// get the key to the suggestions array
		if(strpos($this->name, 'MS ') === 0)
			return preg_replace('/^MS ([^\s]+).*$/', '$1', $this->name);
		$tmp = preg_split('/[\s\-,]/', $this->name);
		return $tmp[0];
	}
	public function getrefname() {
	// generate refname, which should usually be unique with this method
		$authors = $this->getAuthors(array('asArray' => true));
		if(isset($authors[0][0])) {
			$author = $authors[0][0];
		} else {
			$author = '';
		}
		$refname = $author . $this->year . $this->volume . $this->start_page;
		if($refname === '') {
			$refname = $this->title;
		}
		if(is_numeric($refname)) {
			$refname = 'ref' . $refname;
		}
		$refname = str_replace("'", "", $refname);
		return $refname;
	}
	public function getsimpletitle($title = '') {
	// should probably get rid of this
		if(!$title) $title = $this->title;
		$title = preg_replace(
			array(
				'/<\/?i>/u', // |[\'"`*,\.:;\-+()–´«»\/!—]|\s*
				'/[áàâ]/u',
				'/[éèê]/u',
				'/[îíì]/u',
				'/[óôòõ]/u',
				'/[ûúù]/u',
				'/ñ/u',
				'/[çč]/u',
				'/[^a-z0-9]/u',
			),
			array(
				'',
				'a',
				'e',
				'i',
				'o',
				'u',
				'n',
				'c',
				'',
			),
			mb_strtolower($title, mb_detect_encoding($title))
		);
		return $title;
	}
	private function geturl() {
	// get the URL for this file from the data given
		$tries = array(
			'url' => '',
			'doi' => 'http://dx.doi.org.ezp-prod1.hul.harvard.edu/',
			'jstor' => 'http://www.jstor.org.ezp-prod1.hul.harvard.edu/stable/',
			'hdl' => 'http://hdl.handle.net/',
			'pmid' => 'http://www.ncbi.nlm.nih.gov/pubmed/',
			'pmc' => 'http://www.ncbi.nlm.nih.gov/pmc/articles/PMC',
		);
		foreach($tries as $identifier => $url) {
			$try = $this->getIdentifier($identifier);
			if($try !== false) {
				return $url . $try;
			}
		}
		return false;
	}
	public function openurl(array $paras = array()) {
	// open the URL associated with the file
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No parameters accepted */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$url = $this->geturl();
		if($url === false) {
			echo 'No URL to open' . PHP_EOL;
			return false;		
		} else {
			return $this->shell(array(
				'cmd' => 'open',
				'arg' => array($url),
			));
		}
	}
	public function searchgoogletitle() {
	// searches for the title of the article in Google
		$url = 'http://www.google.com/search?q=' . $this->googletitle();
		return $this->shell(array('open', array($url)));
	}
	public function gethost() {
	// get the host part of the URL
		$url = $this->url();
		if($url === false) {
			return false;
		} else {
			return parse_url($url, PHP_URL_HOST);
		}
	}
	public function email(array $paras = array()) {
	// Email this file to someone
	// Inspired by http://www.webcheatsheet.com/PHP/send_email_text_html_attachment.php#attachment
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'to' => 'Address to send to',
				'message' => 'E-mail message',
				'subject' => 'Subject line',
			),
			'default' => array(
				'subject' => $this->name,
			),
			'askifempty' => array(
				'to',
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// not in process_paras because then the code would be evaluated no matter what.
		if(!isset($paras['message'])) {
			$paras['message'] = "<p>Please find attached the following paper:</p>\r\n<ul><li>" . $this->citepaper() . "</li></ul>\r\n";
		}
		// generate boundary hash
		$boundary_hash = md5(date('r', time()));
		$headers = "From: " . FROMADDRESS .
			"\r\nReply-To: " . FROMADDRESS .
			"\r\nContent-Type: multipart/mixed; boundary=\"PHP-mixed-" .
			$boundary_hash . "\"";
		$message = '
--PHP-mixed-' . $boundary_hash . '
Content-Type: multipart/alternative; boundary="PHP-alt-' . $boundary_hash . '"

--PHP-alt-' . $boundary_hash . '
Content-Type: text/html; charset="iso-8859-1"
Content-Transfer-Encoding: 7bit

' . $paras['message'] . '

--PHP-alt-' . $boundary_hash . '--

--PHP-mixed-' . $boundary_hash . '
Content-Type: application/zip; name="' . $this->name . '"
Content-Transfer-Encoding: base64
Content-Disposition: attachment

' . chunk_split(base64_encode(file_get_contents($this->path(array('type' => 'none'))))) . '
--PHP-mixed-' . $boundary_hash . '--
';
		if(!mail($paras['to'], $paras['subject'], $message, $headers)) {
			echo 'Error sending e-mail' . PHP_EOL;
			return false;
		}
		return true;
	}
	public function testNameParser(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($this->isredirect()) {
			return true;
		}
		$parser = $this->getNameParser();
		if($parser->errorOccurred()) {
			$parser->printErrors();
			return false;
		} else {
			//$parser->printParsed();
			return true;
		}
	}
	
	/*
	 * Validators. Should ultimately be organized in some other way.
	 */
	public static function getValidator($field) {
		switch($field) {
			case 'hdl':
				return function($in) {
					return preg_match('/^\d+(\.\d+)?\/(\d+)$/u', $in);
				};
			case 'doi':
				return function($in) {
					return preg_match('/^10\.[A-Za-z0-9\.\/\[\]<>\-;:_()+]+$/u', $in);
				};
			default: 
				return self::getFieldObject($in)->getValidator();
		}
	}

}
