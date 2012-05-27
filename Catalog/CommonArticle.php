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
	public $id; // ID in database
	public $name; //name of file (or handle of citation)
	public $year; //year published
	public $title; //title (chapter title for book chapter; book title for full book or thesis)
	public $series; //journal series
	public $volume; //journal volume
	public $issue; //journal issue
	public $start_page; //start page
	public $end_page; //end page
	public $pages; //number of pages in book
	public $parent; // enclosing article
	public $misc_data; // miscellaneous data

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
				'print' => 'Whether the result should be printed',
			),
			'default' => array(
				'type' => 'none',
				'folder' => false,
				'print' => false,
			),
			'listoptions' => array(
				'type' => array('shell', 'url', 'none'),
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(!$this->isfile()) {
			return false;
		}
		return $this->_path($paras);
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
			$newname = $this->getline('New name: ');
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
	abstract public function isredirect();
	abstract public function resolve_redirect();
	abstract public function issupplement();
	abstract public function isthesis();
	abstract public function thesis_getuni();
	abstract public function thesis_gettype();
	abstract public function supp_getbasic();
	public function isinpress() {
	// checks whether file is in "in press" (and therefore, year etcetera cannot be given)
		return ($this->start_page === 'in press');
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
	public function cite() {
		return $this->__invoke();
	}
	public function __invoke() {
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
		return $this->$func();
	}
	abstract protected function cite_getclass();
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
			if($key === 0) {
				if($paras['firstInitialsBeforeName']) {
					$author = $a[1] . ' ' . $a[0];
				} else {
					$author = $a[0] . ', ' . $a[1];
				}
			} else {
				if($paras['initialsBeforeName']) {
					$author = $a[1] . ' ' . $a[0];
				} else {
					$author = $a[0] . ', ' . $a[1];
				}
			}
			if(isset($a[3])) {
				$author .= ', ' . $a[3];
			}
			$out .= $author;
		}
		return $out;
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
		$class = $this->cite_getclass();
		switch($class) {
			case 'journal':
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
			case 'chapter':
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
			case 'book':
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
		switch($class = $this->cite_getclass()) {
			case 'journal': 
				$temp = 'journal'; 
				break;
			case 'book': case 'chapter': 
				$temp = 'book'; 
				break;
			case 'thesis': 
				$temp = 'thesis'; 
				break;
			case 'unknown': 
				if($out1) {
					return $out1;
				} else {
					return false;
				}
			case 'web': 
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
		switch($class) {
			case 'journal':
				$paras['title'] = $this->title;
				$paras['journal'] = $this->journal;
				$paras['volume'] = $this->volume;
				$paras['issue'] = $this->issue;
				break;
			case 'book':
				$paras['title'] = $this->title;
				if(!$paras['pages']) {
					$paras['pages'] = $this->pages;
				}
				$paras['edition'] = $this->edition;
				break;
			case 'chapter':
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
			case 'thesis':
				$paras['title'] = $this->title;
				$paras['degree'] = $this->thesis_gettype();
				$paras['publisher'] = $this->thesis_getuni();
				$paras['pages'] = $this->pages;
				break;
			case 'web':
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
		switch($class = $this->cite_getclass()) {
			case 'journal':
				$out = $authors . '. ' .
					$this->year . '. ' .
					$this->title . '. ' .
					$this->journal() . ' ' .
					$this->volume . ': ' .
					$this->start_page . '–' .
					$this->end_page . '.';
				break;
			case 'book':
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
		$class = $this->cite_getclass();
		// replace last ; with " &"; others with ","
		$out = '<b>';
		$out .= $this->getAuthors(array(
			'separator' => ',',
			'lastSeparator' => ' &',
		));
		$out .= '</b> ' . $this->year . '. ';
		switch($class) {
			case 'journal':
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
			case 'chapter':
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
			case 'book':
				$out .= '<i>' . $this->title . '.</i>';
				if($this->pages) {
					$out .= ' ' . $this->pages . ' pp.';
				}
				$out .= ' ' . $this->publisher();
				if($this->location()) {
					$out .= ', ' . $this->location();
				}
				$out .= '.';
		}
		// final cleanup
		$out = str_replace(array("  ", ".."), array(" ", "."), $out);
		return $out;
	}
	public function citepalaeontology() {
		$class = $this->cite_getclass();
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
		switch($class) {
			case 'journal':
				$out .= $this->title . '. <i>' . $this->journal() . '</i>, ';
				// TODO: series
				$out .= '<b>' . $this->volume . '</b>, ' . $this->pages();
				break;
			case 'book':
				$out .= '<i>' . $this->title . '.</i> ';
				$out .= $this->publisher() . ', ';
				if($this->location()) {
					$out .= $this->location() . ', ';
				}
				$out .= $this->pages . ' pp.';
				break;
			case 'chapter':
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
			case 'thesis':
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
		$class = $this->cite_getclass();
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
		switch($class) {
			case 'journal':
				$out .= '. ' . $this->journal() . ', ';
				if($this->series) {
					$out .= 'ser. ' . $this->series . ', ';
				}
				$out .= $this->volume . ':' . $this->pages();
				break;
			case 'chapter':
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
			case 'book':
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
		switch($class = $this->cite_getclass()) {
			case 'journal':
				$out .= 'article';
				break;
			case 'book':
				$out .= 'book';
				break;
			case 'chapter':
				$out .= 'incollection';
				break;
			case 'thesis':
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
		switch($class) {
			case 'thesis':
				$add('school', $this->thesis_getuni(), true);
				break;
			case 'journal':
				$add('journal', $this->journal(), true);
				$add('volume', $this->volume);
				$add('number', $this->issue);
				$add('pages', $this->start_page . '--' . $this->end_page);
				break;
			case 'book':
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
		$class = $this->cite_getclass();
		// this is going to be the citation
		$out = '';
		$processauthors = function($in, $type = 'normal') {
			// replace semicolons
			$in = preg_replace(
				array('/;(?=[^;]$)/u', '/;/u'),
				array('& ', ','),
				$in
			);
			if($type === 'editors')
				$in .= ' (Eds)';
			return $in;
		};
		$out .= $this->getAuthors(array(
			'separator' => ',',
			'lastSeparator' => ' &',
		));
		$out .= ' (' . $this->year . ') ';
		switch($class) {
			case 'journal':
				$out .= $this->title . '. <i>' . $this->journal() . '</i>, ';
				$out .= $this->volume . ', ' . $this->pages();
				break;
			case 'chapter':
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
			case 'book':
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
	public function __toString() {
	// return a citation for this file
		$out = $this();
		if(is_array($out))
			return implode(PHP_EOL, $out);
		else
			return $out;
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
				'l' => function($cmd, array $data) use(&$splitTitle, $tolower) {
					if(count($data) !== 2) {
						echo 'Invalid argument' . PHP_EOL;
						return true;
					}
					for($i = $data[0]; $i <= $data[1]; $i++) {
						$splitTitle[$i] = $tolower($splitTitle[$i]);
					}
					return true;
				},
				'u' => function($cmd, array $data) use(&$splitTitle, $toupper) {
					if(count($data) !== 2) {
						echo 'Invalid argument' . PHP_EOL;
						return true;
					}
					for($i = $data[0]; $i <= $data[1]; $i++) {
						$splitTitle[$i] = $toupper($splitTitle[$i]);
					}
					return true;
				},
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
				'r' => function($cmd, array $data) use(&$splitTitle) {
					if(count($data) !== 2) {
						echo 'Invalid argument' . PHP_EOL;
						return true;
					}
					for($i = $data[0]; $i <= $data[1]; $i++) {
						$splitTitle[$i] = '';
					}
					return true;
				},
				'v' => function($cmd, array $data) use(&$splitTitle, $tolower) {
					if(count($data) !== 2) {
						echo 'Invalid argument' . PHP_EOL;
						return true;
					}
					for($i = $data[0]; $i <= $data[1]; $i++) {
						$splitTitle[$i] = preg_replace(
							array('/(?<=[a-z,\.\)])(?=[A-Z])/u', '/(?=\()/u'),
							array(' ', ' '),
							$splitTitle[$i]);
					}
					return true;
				},
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
}
