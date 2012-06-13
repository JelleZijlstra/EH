<?php
require_once(BPATH . '/Catalog/load.php');

trait CommonArticleList {
	/*
	 * Properties
	 */
	public $citetype = 'wp'; // default citation type
	public $verbosecite = true; // whether citation functions need to be verbose
	public $includesfn = true; // whether Sfn needs to be included in Article::citewp()
	public $includerefharv = true; // whether |ref=harv needs to be included in Article::citewp()
	public $addmanual = true; // whether we want data adding functions to ask for manual input
	private $lslist = NULL;
	private $newlist = NULL;
	private $burstlist = NULL;
	public $sugglist = NULL; // list of suggestions used in Article::newadd()
	public $foldertree = NULL; // tree of folders used in the List
	public $foldertree_n = NULL;
	public $pdfcontentcache = array(); // cache for Article::$pdfcontent
	protected static $inform_exclude = array('pdfcontent');
	private static $ArticleList_commands = array(
		'countNameParser' => array('name' => 'countNameParser',
			'desc' => 'Count NameParser results'),
		'renameRegex' => array('name' => 'renameRegex',
			'desc' => 'Rename files that match a regex'),
		'adddata' => array('name' => 'adddata',
			'desc' => 'Add data to existing reference through API lookups',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'addNofile' => array('name' => 'addNofile',
			'aka' => array('a'),
			'desc' => 'Add a new non-file entry',
			'arg' => 'New file handle',
			'execute' => 'callmethod',
			'setcurrent' => true),
		'format' => array('name' => 'format',
			'desc' => 'Format an individual reference',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'echocite' => array('name' => 'echocite',
			'aka' => array('c', 'cite'),
			'desc' => 'Cite a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'addRedirect' => array('name' => 'addRedirect',
			'aka' => array('d'),
			'desc' => 'Add a new redirect',
			'arg' => 'None',
			'execute' => 'callmethod',
			'setcurrent' => true),
		'email' => array('name' => 'email',
			'desc' => 'Email a file to someone',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'findhdl' => array('name' => 'findhdl',
			'desc' => 'Find HDL for AMNH titles',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'searchgoogletitle' => array('name' => 'searchgoogletitle',
			'aka' => array('g'),
			'desc' => 'Search for the file\'s title in Google',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'check' => array('name' => 'check',
			'aka' => array('k'),
			'desc' => 'Check for inconsistencies between the catalog and the library and for files to be added to the library',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'move' => array('name' => 'move',
			'aka' => array('m'),
			'desc' => 'Move a file to a new name',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'openf' => array('name' => 'openf',
			'aka' => array('o', 'open'),
			'desc' => 'Open a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'parse_wlist' => array('name' => 'parse_wlist',
			'aka' => array('p', 'parse'),
			'desc' => 'Parse a string into a reference list',
			'arg' => 'Input string',
			'execute' => 'callmethod'),
		'parse_wtext' => array('name' => 'parse_wtext',
			'aka' => array('tparse'),
			'desc' => 'Parse an input file handle into a Wikipedia-ready file',
			'arg' => 'Filename',
			'execute' => 'callmethod'),
		'remove' => array('name' => 'remove',
			'aka' => array('r'),
			'desc' => 'Remove a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'edittitle' => array('name' => 'edittitle',
			'aka' => array('s'),
			'desc' => 'Edit the title of a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'testNameParser' => array('name' => 'testNameParser',
			'desc' => 'Test the NameParser'),
		'setcitetype' => array('name' => 'setcitetype',
			'aka' => array('t'),
			'desc' => 'set the default style used for citing',
			'arg' => 'citetype',
			'execute' => 'callmethod'),
		'openurl' => array('name' => 'openurl',
			'aka' => array('u'),
			'desc' => 'Open the URL associated with a file',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'removefirstpage' => array('name' => 'removefirstpage',
			'aka' => array('rmfirst'),
			'desc' => 'Remove the first page from the file\'s PDF',
			'arg' => 'File handle',
			'execute' => 'callmethod'),
		'dups' => array('name' => 'dups',
			'aka' => array('duplicates'),
			'desc' => 'Search for duplicate entries',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'temp' => array('name' => 'temp',
			'desc' => 'Temporary cleanup command. Does whatever cleanup it is currently programmed to do.',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'testtitles' => array('name' => 'testtitles',
			'desc' => 'Test Article::findtitle_pdfcontent()\'s capabilities',
			'arg' => 'None',
			'execute' => 'callmethod'),
		'getNameArray' => array('name' => 'getNameArray',
			'desc' => 'Returns an array with the names of all entries'),
	);
	public function cli() {
		$this->setup_commandline('Catalog');
	}
	
	/*
	 * Managing objects
	 */
	public function addEntry(ListEntry $file, array $paras = array()) {
	// Adds a Article to this ArticleList object
	// Type hint is ListEntry instead of Article to keep E_STRICT happy
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'isnew' =>
					'Whether we need to do things we do for new files (as opposed to old ones merely loaded into the catalog)',
			),
			'default' => array(
				'isnew' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		while($this->has($file->name)) {
			echo "File " . $file->name . " already exists.";
			if($this->isredirect($file->name)) {
				echo ' The existing file is a redirect.';
			}
			echo PHP_EOL;
			$cmd = $this->menu(array(
				'options' => array(
					's' => 'skip this file',
					'r' => 'overwrite the existing file',
					'm' => 'rename the new file',
				),
			));
			switch($cmd) {
				case 's': return false;
				case 'r': break 2;
				case 'm':
					$newname = $this->getline('New name of file: ');
					if(!$file->move($newname)) {
						echo 'Error moving file' . PHP_EOL;
						continue 2;
					}
					break;
			}
		}
		parent::addEntry($file);
		if($paras['isnew']) {
			$this->log($file->name, 'Added file to catalog');
			echo "Added to catalog!" . PHP_EOL;
			$this->needsave();
			$this->format($file->name);
		}
		return true;
	}
	public function addNofile(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'handle',
			),
			'checklist' => array(
				'handle' => 'Handle of new entry',
			),
			'askifempty' => array(
				'handle',
			),
			'checkparas' => array(
				'handle' => function($in) {
					if(!is_string($in)) {
						return false;
					}
					// NOFILEs can't have a file extension
					$parser = NameParser::parse($in);
					return ($parser->extension() !== '') && !$this->has($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$childClass = self::$childClass;
		$childClass::makeNewNofile($paras['handle'], $this);
	}
	public function addRedirect(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				// eh returns them in reverse order, though I'd like to change that
				1 => 'handle',
				0 => 'target',
			),
			'checklist' => array(
				'handle' => 'Handle of new redirect',
				'target' => 'Target of new redirect',
			),
			'askifempty' => array('handle', 'target'),
			'checkparas' => array(
				'handle' => function($in) {
					return !$this->has($in);
				},
				'target' => function($in) {
					return $this->has($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$childclass = self::$childClass;
		$childclass::makeNewRedirect($paras['handle'], $paras['target'], $this);
	}
	abstract public function getNameArray(array $paras = array());
	
	/*
	 * check() and its friends
	 */
	private function build_lslist() {
	/*
	 * Gets list of files into $this->lslist, an array of results (Article form).
	 */
		echo "acquiring list of files... ";
		$list = $this->shell(array(
			'cmd' => 'find',
			'arg' => array(
				LIBRARY, '-regex', '.*\.[a-z][a-z]*'
			),
			'printout' => false,
			'return' => 'outputlines',
		));
		$this->lslist = array();
		foreach($list as $line) {
			$line = str_replace(LIBRARY . '/', '', $line);
			$path = explode('/', $line);
			$name = array_pop($path);
			$file = new self::$childClass(
				array($name, $path), 'l', $this
			);
			$this->lslist[$name] = $file;
			/*
			 * Build suggestion lists
			 */
			// full path suggestions
			$key = $file->getkey();
			$folders = implode('/', $path);
			if(isset($this->sugglist[$key])) {
				// if there is already a Suggester, add this file to it
				$this->sugglist[$key]->add($folders);
			} else {
				// else, make a new Suggester
				$this->sugglist[$key] = new Suggester($folders);
			}
			// folder suggestions
			$branch =& $this->foldertree;
			foreach($path as $folder) {
				if(!isset($branch[$folder])) {
					$branch[$folder] = array();
				}
				$branch =& $branch[$folder];
			}
		}
		echo 'done' . PHP_EOL;
		return true;
	}
	private function build_newlist($path = TEMPPATH, $out = 'newlist') {
		// reset out list
		$this->$out = array();
		echo "acquiring list of new files... ";
		// ls output as string
		$list = $this->shell(array(
			'cmd' => 'ls',
			'arg' => array('-p', $path),
			'printout' => false,
			'return' => 'outputlines',
		));
		foreach($list as $file) {
			if(!preg_match("/\/$/", $file) && strlen($file) > 0) {
				$this->{$out}[$file] = new self::$childClass(NULL, 'e', $this);
				$this->{$out}[$file]->name = $file;
			}
		}
		if(count($this->$out) === 0) {
			echo "no new files found" . PHP_EOL;
			return false;
		}
		echo "done" . PHP_EOL;
	}
	public function check(array $paras = array()) {
	/*
	 * Checks the catalog for things to be changed:
	 * - Checks whether there are any files in the catalog that are not in the
	 *   library
	 * - Checks whether there are any files in the library that are not in the
	 *   catalog
	 * - Checks whether there are new files in temporary storage that need to be
	 *   added to the library
	 */
	 	if($this->process_paras($paras, array(
	 		'name' => __FUNCTION__,
	 		'checklist' => array( /* No paras */ ),
	 	)) === PROCESS_PARAS_ERROR_FOUND) return false;
		// always get new ls list, since changes may have occurred since previous check()
		$this->build_lslist();
		if(!$this->lslist) {
			return false;
		}
		$date = new DateTime();
		$this->log(PHP_EOL . PHP_EOL . 'check() session ' . $date->format('Y-m-d H:i:s') . PHP_EOL);
		try {
			// check whether all files in the catalog are in the actual library
			$this->csvcheck();
			// check whether all files in the actual library are in the catalog
			$this->lscheck();
			// check whether there are any files to be burst
			$this->burstcheck();
			// check whether there are any new files to be added
			$this->newcheck();
		} catch(StopException $e) {
			echo 'Exiting from check (' . $e->getMessage() . ')' . PHP_EOL;
			return false;
		}
		return true;
	}
	private function csvcheck() {
	/* check CSV list for problems
	 * - detect articles in catalog that are not in the actual library
	 * - correct filepaths
	 */
		echo 'checking whether cataloged articles are in library... ';
		foreach($this->c as $file) {
			// if file already exists in right place
			if(isset($this->lslist[$file->name])) {
				// update path
				$file->setpath(array('fromfile' => $this->lslist[$file->name]));
			} elseif($file->isfile() && !$file->isredirect()) {
				echo PHP_EOL;
				$cmd = $this->menu(array(
					'head' => 'Could not find file ' . $file->name,
					'options' => array(
						'i' => 'give information about this file',
						'l' => 'file has been renamed',
						'r' => 'remove this file from the catalog',
						'm' => 'move to the next component',
						's' => 'skip this file',
						'q' => 'quit the program',
						'e' => 'edit the file',
					),
					'process' => array(
						'i' => function() use($file) {
							$file->inform();
						},
						'e' => function() use($file) {
							$file->edit();
						},
						'q' => function() {
							throw new StopException('csvcheck');					
						},
						'm' => function(&$cmd) {
							$cmd = true;
							return false;
						},
					),
				));
				switch($cmd) {
					case 'l': 
						$file->effect_rename(array(
							'elist' => $this->c,
							'searchlist' => $this->lslist,
						)); 
						break;
					case 'r':
						$file->log('Removed');
						$this->needsave();
						$this->removeEntry($file->name);
						break;
					case 's':
						break;
					default:
						return $cmd;
				}
			}
		}
		echo 'done' . PHP_EOL;
		return true;
	}
	private function lscheck() {
	/* check LS list for errors
	 * - Detect articles in the library that are not in the catalog.
	 */
		echo "checking whether articles in library are in catalog... ";
		foreach($this->lslist as $lsfile) {
			if(!$this->has($lsfile->name) or $this->isredirect($file->name)) {
				echo PHP_EOL;
				$cmd = $this->menu(array(
					'options' => array(
						'l' => 'file has been renamed',
						'a' => 'add the file to the catalog',
						's' => 'skip this file',
						'q' => 'quit the program',
						'm' => 'move to the next component of the catalog',
					),
					'head' => 
						'Could not find file ' . $lsfile->name . ' in catalog',
				));
				switch($cmd) {
					case 'q': 
						throw new StopException('lscheck');
					case 'm': 
						return true;
					case 'l':
						// don't know what will happen here if there is a redirect
						$lsfile->effect_rename(array(
							'elist' => $this->lslist,
							'searchlist' => $this->c,
							'domove' => true,
						)); 
						break;
					case 's': 
						break;
					case 'a':
						if($this->isredirect($file->name)) {
							// remove redirect
							$this->remove(array(
								$file->name, 
								'force' => true,
							));
						}
						$lsfile->add();
						$this->addEntry($lsfile, array('isnew' => true));
						break;
				}
			}
		}
		echo "done" . PHP_EOL;
		return true;
	}
	private function burstcheck() {
		echo 'checking for files to be bursted... ';
		$this->build_newlist(BURSTPATH, 'burstlist');
		if($this->burstlist) {
			foreach($this->burstlist as $file) {
				$file->burst();
			}
		}
		echo 'done' . PHP_EOL;
		return true;
	}
	private function newcheck() {
	// look for new files
		echo 'checking for newly added articles... ';
		$this->build_newlist();
		if($this->newlist) {
			foreach($this->newlist as $file) {
				$this->addNewFile($file);
			}
		}
		echo 'done' . PHP_EOL;
		return true;
	}
	public function addNewFile(ArticleInterface $file) {
		if($file->newadd(array('lslist' => $this->lslist))) {
			$this->addEntry($file, array('isnew' => true));
		}	
	}
	public function build_sugglist() {
		if($this->sugglist === array()) {
			return $this->build_lslist();
		}
		return true;
	}
	public function build_foldertree() {
		if($this->foldertree === array()) {
			return $this->build_lslist();
		}
		return true;
	}
	
	/*
	 * Operations on files
	 */
	public function setcitetype(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'new',
			),
			'checklist' => array(
				'new' => 'New citetype',
			),
			'askifempty' => array(
				'new',
			),
			'checkparas' => array(
				'new' => array(__CLASS__, 'validcitetype'),
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->citetype = $paras['new'];
		return $this->citetype;
	}
	public static function validcitetype($in) {
		return method_exists(self::$childClass, 'cite' . $in);
	}
	public function testtitles($paras = array()) {
	// Test the findtitle_pdfcontent() method.
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'file' => 'File to write results too',
			),
			'default' => array('file' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($paras['file'])
			$fp = fopen($paras['file'], 'w');
		$matches = $mismatches = $impossible = 0;
		foreach($this->c as $child) {
			$pdftitle = $child->findtitle_pdfcontent();
			if(!$pdftitle or $child->title[0] === '/') {
				$impossible++;
				continue;
			}
			$rectitle = $child->getsimpletitle();
			$dettitle = $child->getsimpletitle($pdftitle);
			if(($dettitle !== $rectitle) and
				($rectitle ? (strpos($dettitle, $rectitle) === false) : true) and
				($dettitle ? (strpos($rectitle, $dettitle) === false) : true)
			) {
				echo 'Title mismatch for file ' . $child->name . PHP_EOL;
				echo 'Levenshtein distance: ' . ($levenshtein = levenshtein($dettitle, $rectitle)) . PHP_EOL;
				echo 'Actual title: ' . $child->title . PHP_EOL;
				echo "\tSimplified as $rectitle\n";
				echo "\tSimplified as $dettitle\n";
				echo 'Detected title: ' . $pdftitle . PHP_EOL;
				$mismatches++;
				if($paras['file'])
					fputcsv($fp, array($child->name, $child->title, $pdftitle, $rectitle, $dettitle, $levenshtein));
			}
			else {
				$matches++;
			}
		}
		if($paras['file'])
			fclose($fp);
		echo "\n\nTotal matches: $matches\nTotal mismatches: $mismatches\nCould not determine title: $impossible\n";
	}
	public function getpdfcontentcache() {
		if(count($this->pdfcontentcache) !== 0) return false;
		// this may take huge amounts of memory...
		ini_set('memory_limit', 1e10);
		$this->pdfcontentcache = json_decode(file_get_contents(PDFCONTENTCACHE), true);
		if($this->pdfcontentcache === NULL) {
			echo 'Error retrieving PDF content cache' . PHP_EOL;
			return false;
		}
		return true;
	}
	public function putpdfcontentcache() {
		// only save if we've actually retrieved the cache
		if(count($this->pdfcontentcache) > 0) {
			file_put_contents(
				PDFCONTENTCACHE,
				json_encode($this->pdfcontentcache)
			);
		}
	}	
	public function countNameParser(array $paras) {
		$count = 0;
		$good = 0;
		$this->each(function($e) use(&$count, &$good) {
			$count++;
			if($e->testNameParser()) {
				$good++;
			}
		});
		echo $good . ' of ' . $count . ' (' . ($good / $count * 100) . '%)' 
			. PHP_EOL;
	}
	public function renameRegex(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				0 => 'from',
				1 => 'to',
				'f' => 'force',
			),
			'checklist' => array(
				'from' => 'Regex to recognize faulty titles',
				'to' => 'Replacement',
				'force' => 'Do not ask for confirmation',
			),
			'checkparas' => array(
				'from' => function($in) {
					return ($in[0] === '/') and !preg_match('/\/[^\/]*e[^\/]*$/', $in);
				},
			),
			'errorifempty' => array('from', 'to'),
			'default' => array('force' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->each(function($e) use($paras) {
			if($e->isredirect()) {
				return;
			}
			if(preg_match($paras['from'], $e->name)) {
				$newTitle = preg_replace($paras['from'], $paras['to'], $e->name);
				if($paras['force']) {
					$cmd = true;
				} else {
					$cmd = $this->ynmenu(
						'Rename "' . $e->name . '" to "' . $newTitle . '"? ');
				}
				if($cmd) {
					$e->move($newTitle);
				}
			}
		});
		return true;
	}
}

class Suggester {
// cf. ArticleList::$sugglist
	private $suggestions = array();
	function __construct($folders) {
		$this->suggestions[$folders] = 1;
	}
	function add($folders) {
		if(!isset($this->suggestions[$folders]))
			$this->suggestions[$folders] = 0;
		$this->suggestions[$folders]++;
	}
	function sort() {
		arsort($this->suggestions);
	}
	function getsugg() {
		$this->sort();
		foreach($this->suggestions as $key => $count) {
			$out[] = explode('/', $key);
		}
		return $out;
	}
}
