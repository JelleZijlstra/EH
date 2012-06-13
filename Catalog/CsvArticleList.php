<?php

class CsvArticleList extends CsvContainerList {
	use CommonArticleList;

	protected static $fileloc = CATALOG;
	protected static $logfile = CATALOG_LOG;
	protected static $childClass = 'CsvArticle';
	/* core utils */
	protected function __construct(array $commands = array()) {
		parent::__construct(self::$ArticleList_commands);
	}
	public function getNameArray(array $paras = array()) {
		if(!$this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		))) return false;
		$out = array();
		foreach($this->c as $file) {
			if(!$file->isredirect()) {
				$out[] = $file->name;
			}
		}
		return $out;
	}
	private $autocompleter = NULL;
	public function getAutocompleter() {
		if($this->autocompleter === NULL) {
			$this->autocompleter = 
				new NaiveAutocompleter($this->getNameArray());
		}
		return $this->autocompleter;
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
	private function find_dups($key, $needle, array $paras = array()) {
		if(!isset($paras['quiet'])) {
			$paras['quiet'] = true;
		}
		$paras[$key] = $needle;
		$files = $this->bfind($paras);
		if(($files === false) or (count($files) === 0)) {
			return false;
		}
		foreach($files as $file) {
			echo $file->name . PHP_EOL . $file->citepaper() . PHP_EOL;
		}
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
					if($this->has($fileName)) {
						$fileName = $this->get($fileName)->resolve_redirect();
					}
					// if there is no file or the redirect is broken
					if($fileName === false) {
						return false;
					}
					$file = $this->get($fileName);
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
					$newname = $this->menu(array(
						'head' => 'New name of file: ',
						'options' => array('q' => 'Quit'),
						'validfunc' => function($in) {
							return !$this->has($in);
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
					$this->needsave();				
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
						$target = $this->menu(array(
							'head' => 'Type the redirect target below',
							'options' => array('q' => 'Quit this file'),
						));
						if($target === 'q') break;
					}
					$this->addEntry(
						new self::$childClass(array($fileName, $target), 'r', $this)
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
}
