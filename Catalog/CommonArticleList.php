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
		'add_nofile' => array('name' => 'add_nofile',
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
		'listinfo' => array('name' => 'listinfo',
			'desc' => 'Give information about the ArticleList object',
			'arg' => 'None',
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
	);
	
	/*
	 * Managing objects
	 */
	abstract public function makeredirect($handle, $target);
	abstract public function addRedirect(array $paras);
	
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
			if(!$this->has($lsfile->name)) {
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
						$lsfile->effect_rename(array(
							'elist' => $this->lslist,
							'searchlist' => $this->c,
							'domove' => true,
						)); 
						break;
					case 's': 
						break;
					case 'a':
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
				switch($file->newadd(array('lslist' => $this->lslist))) {
					case 0: 
						return true;
					case 1: 
						continue 2;
					case 2: 
						$this->addEntry($file, array('isnew' => true));
						break;
				}
			}
		}
		echo 'done' . PHP_EOL;
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
