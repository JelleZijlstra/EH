<?php
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Catalog/load.php');

class ArticleList extends CsvContainerList {
	public $citetype; // default citation type
	public $verbosecite; // whether citation functions need to be verbose
	public $includesfn; // whether Sfn needs to be included in Article::citewp()
	public $includerefharv; // whether |ref=harv needs to be included in Article::citewp()
	public $addmanual; // whether we want data adding functions to ask for manual input
	private $lslist;
	private $newlist;
	private $burstlist;
	public $sugglist; // list of suggestions used in Article::newadd()
	public $foldertree; // tree of folders used in the List
	public $foldertree_n;
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
		'add_redirect' => array('name' => 'add_redirect',
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
	protected static $fileloc = CATALOG;
	protected static $logfile = CATALOG_LOG;
	protected static $childClass = 'Article';
	/* core utils */
	protected function __construct(array $commands = array()) {
		parent::__construct(self::$ArticleList_commands);
		// set some stuff initially
		$this->citetype = 'wp';
		$this->verbosecite = true;
		$this->includesfn = true;
		$this->includerefharv = true;
		$this->addmanual = true;
	}
	public function makeredirect($handle, $target) {
	// redirect one file to another
		if(!$this->has($handle)) {
			return false;
		}
		$redirected = $this->get($handle);
		$redirected->name = 'SEE ' . $target;
		$redirected->format();
		return true;
	}
	/* load related lists */
	private function build_lslist() {
	/*
	 * Gets list of files into $this->lslist, an array of results (Article form).
	 */
		echo "acquiring list of files... ";
		// ls output as string
		$list = $this->shell(array(
			'cmd' => 'ls',
			'arg' => array('-pR', LIBRARY),
			'printout' => false,
			'return' => 'output',
		));
		if(!$list) {
			echo "Could not find library." . PHP_EOL;
			return false;
		}
		// output associative array
		$this->lslist = array();
		$escapelibrary = preg_replace("/\//", "\/", LIBRARY);
		$list = preg_split("/\n" . PHP_EOL . $escapelibrary . "/", $list);
		foreach($list as $folder) {
			$folder = preg_split("/:\n/", $folder);
			if(!isset($folder[1])) {
				continue;
			}
			$path = preg_split("/\//", $folder[0]);
			if(!isset($path[2])) {
				$path[2] = '';
			}
			if(!isset($path[3])) {
				$path[3] = '';
			}
			$filelist = preg_split("/\n/", $folder[1]);
			foreach($filelist as $file) {
				// do not handle directories
				if(!preg_match("/\/$/", $file) && $file) {
					$this->lslist[$file] = new Article(
						array($file, $path[1], $path[2], $path[3]), 'l', $this
					);
				}
			}
		}
		echo "done" . PHP_EOL;
	}
	private function build_newlist($path = '', $out = 'newlist') {
		// why do we need this?
		if($out === 'p') {
			return false;
		}
		// reset out list
		$this->$out = array();
		echo "acquiring list of new files... ";
		if($path === '') $path = TEMPPATH;
		// ls output as string
		$list = $this->shell(array(
			'cmd' => 'ls',
			'arg' => array('-p', $path),
			'printout' => false,
			'return' => 'output',
		));
		if($list === '') {
			echo "no new files found" . PHP_EOL;
			return false;
		}
		// output associative array
		$list = preg_split("/\n/", $list);
		foreach($list as $file) {
			if(!preg_match("/\/$/", $file) && $file) {
				$this->{$out}[$file] = new Article(NULL, 'e', $this);
				$this->{$out}[$file]->name = $file;
			}
		}
		if(count($this->$out) === 0) {
			echo "no new files found" . PHP_EOL;
			return false;
		}
		echo "done" . PHP_EOL;
	}
	/* adding stuff to the list */
	public function addEntry(ListEntry $file, array $paras = array()) {
	// Adds a Article to this ArticleList object
	// Type hint is ListEntry instead of Article to keep E_STRICT happy
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'name' =>
					'Filename to write under (if different from $file->name',
				'isnew' =>
					'Whether we need to do things we do for new files (as opposed to old ones merely loaded into the catalog)',
			),
			'default' => array(
				'name' => $file->name,
				'isnew' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		while($this->has($paras['name'])) {
			echo "File " . $paras['name'] . " already exists.";
			if($this->isredirect($file->name)) echo ' The existing file is a redirect.';
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
					$newname = $this->getline(array(
						'prompt' => 'New name of file: '
					));
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
	public function add_nofile(array $paras = array()) {
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
					// NOFILEs can't have dots in their names
					if(strpos($in, '.') !== false) {
						return false;
					}
					// when we're in PHP 5.4, use $this->has instead here
					return !ArticleList::singleton()->has($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->addEntry(
			Article::makeNofile($paras['handle'], $this),
			array('isnew' => true)
		);
	}
	public function add_redirect(array $paras = array()) {
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
					return !ArticleList::singleton()->has($in);
				},
				'target' => function($in) {
					return ArticleList::singleton()->has($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return $this->addEntry(
			new Article(array($paras['handle'], $paras['target']), 'r', $this),
			array('isnew' => true)
		);
	}
	/* parsing - needs overall revision, and coordination with Parser class */
	protected function parse_wlist(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'file'),
			'default' => array('file' => 'File to parse'),
			'errorifempty' => array('file'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		return parse_wlist($paras['file']);
	}
	protected function parse_wtext(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(0 => 'file'),
			'default' => array('file' => 'File to parse'),
			'errorifempty' => array('file'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if(strpos($paras['file'], '/') === false)
			$paras['file'] = '/Users/jellezijlstra/Dropbox/Open WP/' . $paras['file'];
		return parse_wtext($paras['file']);
	}
	/* do things with files */
	public function cli() {
	// Performs various functions in a pseudo-command line. A main entry point.
		$this->setup_commandline('Catalog');
	}
	public function listinfo() {
		foreach($this as $property => $value) {
			echo $property . ': '; echo $value . PHP_EOL;
		}
	}
	/* check */
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
	private function find_dups($key, $needle, array $paras = array()) {
		if(!isset($paras['quiet']))
			$paras['quiet'] = true;
		$paras[$key] = $needle;
		$files = $this->bfind($paras);
		if(($files === false) or (count($files) === 0))
			return false;
		foreach($files as $file)
			echo $file->name . PHP_EOL . $file->citepaper() . PHP_EOL;
		return $files;
	}
	public function dups(array $paras = array()) {
	// automatically find (some) duplicate files, and handle them
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		/*
		 * First try: find duplicate DOIs
		 */
		$dois = $this->mlist(array('field' => 'doi', 'print' => false));
		foreach($dois as $doi => $n) {
			if($n > 1 && $doi) {
				echo "Found $n instances of DOI $doi" . PHP_EOL;
				$files = $this->find_dups('doi', $doi);
				if($files === false) {
					continue;
				}
				if(!$this->dups_core($files)) {
					return;
				}
			}
		}
		/*
		 * Other tries: titles?
		 */
		$titles = $this->mlist(array(
			'field' => 'getsimpletitle',
			'isfunc' => true,
			'print' => false
		));
		if($titles === false) {
			return;
		}
		foreach($titles as $title => $n) {
			if($n > 1 && $title) {
				echo "Found $n instances of title $title" . PHP_EOL;
				$files = $this->find_dups('getsimpletitle()', $title);
				if($files === false) {
					continue;
				}
				if(!$this->dups_core($files)) {
					return;
				}
			}
		}
	}
	private function dups_core($files) {
		return $this->menu(array(
			'head' => 'dups> ',
			'headasprompt' => true,
			'processcommand' => function(&$cmd, &$data) {
				if(strlen($cmd) > 2) {
					$data = array();
					$fileName = substr($cmd, 2);
					$list = ArticleList::singleton();
					if($list->has($fileName)) {
						$fileName = $list->get($fileName)->resolve_redirect();
					}
					// if there is no file or the redirect is broken
					if($fileName === false) {
						return false;
					}
					$file = $list->get($fileName);
					$data['fileName'] = $fileName;
					$data['file'] = $file;
					$cmd = substr($cmd, 0, 1);
				}
				if($cmd === 'e' || $cmd === 'm' || $cmd === 'r') {
					if($data === NULL) {
						return false;
					}
				}
				return $cmd;
			},
			'options' => array(
				'e' => 'Edit file <file>',
				'm' => 'Move file <file>',
				'r' => 'Remove file <file>',
				's' => 'Quit this duplicate set',
				'q' => 'Quit this function',
				'o' => 'Open the files',
				'i' => 'Give information about the files',
			),
			'process' => array(
				'e' => function($cmd, $data) {
					$data['file']->edit();
					return true;
				},
				'm' => function($cmd, $data) {
					$newname = ArticleList::singleton()->menu(array(
						'head' => 'New name of file: ',
						'options' => array('q' => 'Quit'),
						'validfunc' => function($in) {
							return !ArticleList::singleton()->has($in);
						},
						'process' => array(
							'q' => function(&$cmd) { 
								$cmd = false; 
								return false; 
							},
						),
					));
					if($newname === false) {
						break;
					}
					$data['file']->move($newname);
					ArticleList::singleton()->needsave();				
					return true;
				},
				'o' => function() use($files) {
					foreach($files as $file) {
						$file->openf();
					}
					return true;
				},
				'i' => function() use($files) {
					foreach($files as $file) {
						$file->inform();
					}
					return true;
				},
				'r' => function($cmd, $data) use($files) {
					$data['file']->remove();
					$list = ArticleList::singleton();
					// add redirect from old file
					$targets = array();
					foreach($files as $file) {
						if($file->name and ($file->name !== $data['file'])) {
							$targets[] = $fileo->name;
						}
					}
					if(count($targets) === 1) {
						$target = array_pop($targets);
					} else {
						echo 'Could not determine redirect target. Options:' 
							. PHP_EOL;
						foreach($targets as $target) {
							echo '- ' . $target . PHP_EOL;
						}
						$target = $list->menu(array(
							'head' => 'Type the redirect target below',
							'options' => array('q' => 'Quit this file'),
						));
						if($target === 'q') break;
					}
					$list->addEntry(
						new Article(array($fileName, $target), 'r', $list)
					);
					return true;
				},
				's' => function(&$cmd) {
					$cmd = true;
					return false;
				},
				'q' => function(&$cmd) {
					$cmd = false;
					return false;
				}
			),
		));
	}
	public function stats(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array(
				'f' => 'includefoldertree',
			),
			'checklist' => array(
				'includefoldertree' =>
					'Whether we need to print the foldertree',
			),
			'default' => array(
				'includefoldertree' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$results = array();
		$nredirects = $nnonfiles = 0;
		$this->each(function($file) use(&$results, &$nnonfiles, &$nredirects) {
			if($file->isredirect()) {
				$nredirects++;
			} else {
				foreach($file as $key => $property) {
					if($property) {
						if(!isset($results[$key])) {
							$results[$key] = 0;
						}
						$results[$key]++;
					}
					if(is_array($property)) {
						foreach($property as $key => $prop) {
							if($prop) {
								if(!isset($results[$key])) {
									$results[$key] = 0;
								}
								$results[$key]++;
							}
						}
					}
				}
				if(!$file->isfile()) {
					$nnonfiles++;
				}
			}
		});
		$total = $this->count();
		echo 'Total number of files is ' . $total . '. Of these, ' 
			. $nredirects . ' are redirects and ' 
			. $nnonfiles . ' are not actual files.' . PHP_EOL;
		$total -= $nredirects;
		ksort($results);
		foreach($results as $field => $number) {
			echo $field . ': ' . $number . ' of ' . $total . ' (' 
				. round($number/$total*100, 1) . '%)' . PHP_EOL;
		}
		if($paras['includefoldertree']) {
			$this->build_foldertree_n();
			foreach($this->foldertree_n as $folder => $fc) {
				echo $folder . ': ' . $fc[0] . PHP_EOL;
				foreach($fc as $sfolder => $sfc) {
					if(!$sfolder) continue;
					echo $folder . '/' . $sfolder . ': ' . $sfc[0] . PHP_EOL;
					foreach($sfc as $ssfolder => $ssfc) {
						if(!$ssfolder) continue;
						echo $folder . '/' . $sfolder . '/' . $ssfolder . ': ' 
							. $ssfc . PHP_EOL;
					}
				}
			}
		}
		return;
	}
	/* citing */
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
				'new' => function($in) {
					return ArticleList::validcitetype($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->citetype = $paras['new'];
		return $this->citetype;
	}
	public static function validcitetype($in) {
		return method_exists('Article', 'cite' . $in);
	}
	/* first kind of suggestions: full paths */
	public function build_sugglist() {
	// builds an array of Suggester objects, with $file->getkey() (currently first word of the filename) used as the array key.
		foreach($this->c as $file) {
			// get information
			$folders = $file->folderstr();
			$key = $file->getkey();
			// if there is already a Suggester for this object, add the current folderstr to it
			if(isset($this->sugglist[$key]))
				$this->sugglist[$key]->add($folders);
			// else, make a new Suggester
			else
				$this->sugglist[$key] = new Suggester($folders);
		}
	}
	/* 2nd kind of suggestions: list folders with IDs */
	public function build_foldertree() {
	// build a tree of folders (array of array of arrays) used for suggestions
		foreach($this->c as $file) {
			//exclude non-files and redirects
			if($file->isor('nofile', 'redirect')) continue;
			if(!isset($this->foldertree[$file->folder]))
				$this->foldertree[$file->folder] = array();
			if(!isset($this->foldertree[$file->folder][$file->sfolder]) and $file->sfolder)
				$this->foldertree[$file->folder][$file->sfolder] = array();
			if(!isset($this->foldertree[$file->folder][$file->sfolder][$file->ssfolder]) and $file->ssfolder)
				$this->foldertree[$file->folder][$file->sfolder][$file->ssfolder] = array();
		}
	}
	private function build_foldertree_n() {
	// as build_foldertree(), but include number of files
		foreach($this->c as $file) {
			// exclude non-files and redirects
			if($file->isor('nofile', 'redirect')) continue;
			// we'll use these so much, the short forms will be easier
			$f = $file->folder;
			$sf = $file->sfolder;
			$ssf = $file->ssfolder;
			if(!isset($this->foldertree_n[$f])) {
				$this->foldertree_n[$f] = array(0 => 0);
				if(!$sf) $this->foldertree_n[$f][0] = 1;
			}
			else if(!$sf)
				$this->foldertree_n[$f][0]++;
			if(!isset($this->foldertree_n[$f][$sf]) and $sf) {
				$this->foldertree_n[$f][$sf] = array(0 => 0);
				if(!$ssf) $this->foldertree_n[$f][$sf][0] = 1;
			}
			else if($sf && !$ssf)
				$this->foldertree_n[$f][$sf][0]++;
			if($ssf) {
				if(!isset($this->foldertree_n[$f][$sf][$ssf]))
					$this->foldertree_n[$f][$sf][$ssf] = 0;
				$this->foldertree_n[$f][$sf][$ssf]++;
			}
		}
	}
	/* URL magic */
	private $urls_by_journal;
	public function geturlsjournal($journal, $paras = '') {
		if(!$journal)
			return false;
		if(!$paras['rebuild'] and !isset($this->urls_by_journal)) {
			$this->urls_by_journal = $this->mlist(array(
				'field' => 'gethost',
				'isfunc' => true,
				'groupby' => 'journal', 
				'print' => false, 
				'printresult' => false,
			));
		}
		if(!is_array($this->urls_by_journal)) return false;
		return $this->urls_by_journal[$journal] ?: false;
	}
	public function temp() {
	// general cleanup/test function; does whatever it is currently programmed to do (which at the moment is nothing)
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
	
	/*
	 * SqlContainerList things.
	 */
	public function table() {
		return 'article';
	}
}
class Suggester {
// cf. ArticleList::$sugglist
	private $suggestions;
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
		foreach($this->suggestions as $key => $count)
			$out[] = explode(':', $key);
		return $out;
	}
}
