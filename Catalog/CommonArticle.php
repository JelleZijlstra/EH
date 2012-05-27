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
	abstract public function issupplement();
	public function isinpress() {
	// checks whether file is in "in press" (and therefore, year etcetera cannot be given)
		return ($this->start_page === 'in press');
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
