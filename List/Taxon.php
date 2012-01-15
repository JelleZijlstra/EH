<?php
class Taxon extends ListEntry {
	public $rank;
	public $name;
	public $authority;
	public $movedgenus;
	public $year;
	public $parent;
	public $comments;
	public $children;
	public $endemic;
	static $n_endemic = array('country', 'sub');
	public $range;
	public $status; // String. May be any valid IUCN Red List status (EX, EW, CR, EN, VU, DD, NT, LC), Not Evaluated (NE), Prehistoric (PH; that is, extinct and Holocene, but too old to be on the Red List), or FO (fossil; went extinct before the Holocene)
	public $ids;
	static $n_ids = array('iucn');
	public $misc;
	static $n_misc = array('rawr', 'originalref');
	public $props;
	static $parentlist = 'taxonlist';
	protected static $arrays_to_check = array('ids', 'endemic', 'misc');
	public $ngen;
	public $nspec;
	public $isextant;
	public $citation; // citation to the original description
	protected static $Taxon_commands = array(
		'populatecitation' => array('name' => 'populatecitation',
			'desc' => 'Attempt to populate the citation field',
			'aka' => 'addcite',
			'arg' => 'None',
			'execute' => 'callmethod',
		),
	);
	function __construct($in, $code) {
	// $in: input data (array or string)
	// $code: kind of FullFile to make
		global $csvlist;
		if($csvlist) $this->p = $csvlist;
		if(!$code) return;
		switch($code) {
			case 'f': // loading from file
				if(!is_array($in)) {
					trigger_error('Invalid input to ' . __METHOD__, E_USER_NOTICE);
					return;
				}
				/* Elements of class Taxon are stored in CSV as follows:
				NAME,AUTHORITY,MOVEDGENUS,YEAR,RANK,PARENT,COMMENTS,STATUS,CITATION,ENDEMIC [as JSON'd array],RANGE [as JSON'd array],IDS [as serialized array, not JSON'd because of better object handling in serialize],MISC [as JSON'd array, miscellaneous properties]
				*/
				$this->name = $in[0];
				$this->authority = $in[1];
				$this->movedgenus = $in[2];
				$this->year = $in[3];
				$this->rank = $in[4];
				$this->parent = $in[5];
				$this->comments = $in[6];
				$this->status = $in[7];
				$this->citation = $in[8];
				$this->endemic = json_decode($in[9], true);
				if($in[9]) $this->range = unserialize($in[10]);
				if($in[10]) $this->temprange = unserialize($in[11]);
				$this->ids = json_decode($in[12], true);
				$this->misc = json_decode($in[13], true);
				break;
			case 'n': // new file
				$this->name = $in;
				$this->newadd();
				break;
		}
	}
	function toarray() {
		$out = array();
		$out[] = $this->name;
		$out[] = $this->authority;
		$out[] = $this->movedgenus;
		$out[] = $this->year;
		$out[] = $this->rank;
		$out[] = $this->parent;
		$out[] = $this->comments;
		$out[] = $this->status;
		$out[] = $this->citation;
		$out[] = $this->getarray('endemic');
		if($this->range)
			$out[] = serialize($this->range);
		else
			$out[] = NULL;
		if($this->temprange)
			$out[] = serialize($this->temprange);
		else
			$out[] = NULL;
		$out[] = $this->getarray('ids');
		$out[] = $this->getarray('misc');
		return $out;
	}
	function move($newname) {
	// TODO
		if(!$this->p->moveinlist($this->name, $newname))
			return false;
		$this->name = $newname;
		return true;
	}
	public function format() {
		/* FORMATTING */
		$this->comments = trim($this->comments);
		/* WARNINGS */
		if(!$this->p->has($this->parent))
			$this->warn('Non-existent taxon', 'parent');
		if($this->rank === 'species' and !$this->status)
			$this->warn('No content for species', 'status');
	}
	public function set($paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'easy' => 
					'Use easy mode: do not check for necessary concurrent changes',
				'verbose' =>
					'Print each property that is changed',
			),
			'checkfunc' => function($in) {
				return true;
			},
			'default' => array(
				'easy' => false,
				'verbose' => false,
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		foreach($paras as $field => $content) {
			if(self::hasproperty($field)) {
				if($paras['easy'])
					$this->$field = $content;
				else switch($field) {
					case 'parent':
						if(!$this->p->has($content))
							return false;
						if($this->rank === 'species') {
							$newname = str_replace($this->parent . ' ', $content . ' ', $this->name);
							if($this->p->has($newname))
								return false;
							$this->set(array('name' => $newname));
						}
						unset($this->p->par[$this->parent][$this->name]);
						$this->p->par[$content][$this->name] = $this->name;
						$this->parent = $content;
						$this->setranktosisters();
						$this->setnametorank();
						// need stuff changing the name and rank for family-groups
						break;
					case 'name':
						$oldname = $this->name;
						$this->name = $content;
						$this->p->add_entry($this);
						$this->p->remove($oldname);
						break;
					default:
						$this->$field = $content;
						break;
				}
				if($paras['verbose']) 
					echo 'Set ' . $field . ' to "' . $content . '".' . PHP_EOL;
				$this->p->needsave();
			}
		}
		return true;
	}
	public function getchildren() {
		if(!isset($this->p->par[$this->name]))
			return NULL;
		else
			return $this->p->par[$this->name];
	}
	private function call_children($func, array $paras) {
		if(!isset($this->p->par[$this->name])) return;
		$children = $this->p->par[$this->name];
		foreach($children as $child) {
			$this->p->$func($child, $paras);
		}
	}
	public function html(array $paras = array()) {
	// print taxon
		// output file stream; should be set before calling this function
		$out = $this->p->html_out;
		// write data for this taxon
		$tmp = "<" . $this->rank . ">" . PHP_EOL;
		$tmp .= "\t<name>" . $this->name . "</name>" . PHP_EOL;
		if($this->authority)
			$tmp .= "\t<authority>" . $this->constructauthority() . "</authority>" . PHP_EOL;
		if($this->originalref)
			$tmp .= '\t<originalref>' . $this->originalref . '</originalref>' . PHP_EOL;
		if($this->comments)
			$tmp .= "\t<comments>Comments: " . $this->comments . "</comments>" . PHP_EOL;
		if($this->country)
			$tmp .= "\t<endemiccountry>Country where this taxon is endemic: " . $this->country . "</endemiccountry>" . PHP_EOL;
		if($this->sub)
			$tmp .= "\t<endemicsubnation>Sub-national region where this taxon is endemic: " . $this->sub . "</endemicsubnation>" . PHP_EOL;
		if($this->nspec and $this->rank !== 'species')
			$tmp .= "\t<nspec>Number of species: " . $this->nspec . "</nspec>" . PHP_EOL;
		if($this->ngen and $this->rank !== 'genus' and $this->rank !== 'species')
			$tmp .= "\t<ngen>Number of genera: " . $this->ngen . "</ngen>" . PHP_EOL;
		fwrite($out, $tmp);
		// print children
		if($children = $this->getchildren()) foreach($children as $child)
			$this->p->html($child);
		fwrite($out, "</" . $this->rank . ">" . PHP_EOL);
	}
	public function text() {
		if($this->rank !== 'genus' and $this->rank !== 'species') {
			return false;
		}
		if($this->rank === 'genus') {
			echo 'Report for taxon ' . $this->name . PHP_EOL;
			$pre = '';
		}
		else
			$pre = "\t";
		$out .= PHP_EOL . $pre . ucfirst($this->rank) . ': ' . $this->name . PHP_EOL;
		$out .= $pre . 'Authority: ' . $this->constructauthority() . PHP_EOL;
		if($this->originalref)
			$out .= $pre . 'Original reference: ' . $this->parse('originalref', 'simple') . PHP_EOL;
		if($this->comments)
			$out .= $pre . 'Comments: ' . $this->parse('comments', 'refend');
		else
			$out .= PHP_EOL;
		if($children = $this->getchildren()) foreach($children as $child) {
			$out .= $this->p->text($child);
		}
		return $out;
	}
	public function wiki($paras) {
		if(!$this->isextant()) return;
		switch($this->rank) {
			// omit some taxa
			case 'class': case 'suborder': case 'infraorder': case 'parvorder': case 'superfamily': break;
			case 'order': $tmp = '=Order [[' . $this->name . ']] ' . $this->constructauthority() . '=' . PHP_EOL; break;
			case 'family': $tmp = '==Family [[' . $this->name . ']] ' . $this->constructauthority() . '==' . PHP_EOL; break;
			case 'subfamily': $tmp = '===Subfamily [[' . $this->name . ']] ' . $this->constructauthority() . '===' . PHP_EOL; break;
			case 'tribe': $tmp = '====Tribe [[' . $this->name . ']] ' . $this->constructauthority() . '====' . PHP_EOL; break;
			case 'subtribe': $tmp = '=====Subtribe [[' . $this->name . ']] ' . $this->constructauthority() . '=====' . PHP_EOL; break;
			case 'genus': $tmp = ";''[[" . $this->name . "]]'' " . $this->constructauthority() . PHP_EOL; break;
			case 'species': $tmp = "*''[[" . $this->name . "]]'' " . $this->constructauthority() . PHP_EOL; break;
		}
		if(in_array($this->name, TaxonList::$separate_wiki) and $this->name !== $paras['taxon']) {
			$this->p->outputwiki(array('taxon' => $this->name));
			fwrite($this->p->wiki_out[$paras['taxon']], $tmp . "See [[/$this->name]] for this large $this->rank." . PHP_EOL);
		}
		else {
			if(isset($tmp)) {
				if($this->comments) {
					$tmp .= ':::<small>Comments: ';
					$tmp .= $this->parse('comments', 'wref');
					$tmp .= '</small>' . PHP_EOL;
				}
				if($this->ngen and ($this->rank !== 'genus'))
					$tmp .= ':::<small>Genera: ' . $this->ngen . '</small>' . PHP_EOL;
				if($this->nspec)
					$tmp .= ':::<small>Species: ' . $this->nspec . '</small>' . PHP_EOL;
				fwrite($this->p->wiki_out[$paras['taxon']], $tmp);
			}
			$this->call_children('wiki', array('taxon' => $paras['taxon']));
		}
	}
	private function parse($para, $mode) {
		if(!self::hasproperty($para)) {
			echo 'No such property: ' . $para . PHP_EOL;
			return false;
		}
		$pname = $mode . '_p';
		if(!isset($this->p->$pname))
			$this->p->$pname = new Parser($mode);
		return $this->p->{$pname}->__invoke($this->$para) ?: false;
	}
	public function completedata() {
	// complete ngen and nspec; calls itself for each child
		// things will go wrong when completedata() is called repeatedly, unless we do this
		$this->ngen = 0;
		$this->nspec = 0;
		$this->isextant = false;
		// don't sort orders. Indeed, no sorting at all for now until I figure out how to do that under the new scheme
		// if($this->rank !== "class") ksort($this->children);
		switch($this->rank) {
			case "genus":
				$this->ngen = 1;
				$children = $this->getchildren();
				if(!$children) break;
				$this->sortchildren();
				foreach($children as $child) {
					if($this->p->get($child, 'status') !== 'FO') {
						$this->nspec++;
						$this->isextant = true;
					}
				}
				break;
			case "species":
				$this->movedgenus = $this->movedgenus ? 1 : '';
				break;
			default:
				$children = $this->getchildren();
				if(!$children) break;
				// don't sort the orders
				if($this->rank !== 'class') $this->sortchildren();
				foreach($children as $child) {
					$this->p->completedata($child);
					if($this->p->get($child, 'isextant') === true) {
						$this->ngen += $this->p->get($child, 'ngen');
						$this->nspec += $this->p->get($child, 'nspec');
						$this->isextant = true;
					}
				}
				break;
		}
		return true;
	}
	private function constructauthority() {
	// construct canonical authority
		if(!$this->authority)
			return '';
		if($this->movedgenus)
			$out = '(';
		else
			$out = '';
		$out .= $this->authority;
		$out .= ', ';
		$out .= $this->year;
		if($this->movedgenus)
			$out .= ')';
		return $out;
	}
	function expandiucn($force = false) {
		if(!$this->iucn) return true;
		if($this->rawr && !$force) return true;
		$url = 'http://api.iucnredlist.org/details/' . $this->iucn . '/0';
		$rla = fopen($url, 'r');
		if(!$rla) {
			echo 'Could not open Red List data: ' . $this->name . PHP_EOL;
			return false;
		}
		echo 'Expanding Red List data for ' . $this->name . PHP_EOL;
		while($line = fgets($rla)) {
/*			if($line === '<ul class=\'country_distribution\'>\n') {
				$uls[] = 'country_distribution';
			}
			if($line === '<ul class=\'countries\'>\n')
			if($line === '</ul>\n') {
				$pop = array_pop($uls);
				if($pop === 'country_distribution') break;
			}*/
			if(preg_match("/<div class='distribution_type'>/", $line))
				$type = trim(str_replace(array("<div class='distribution_type'>", '</div>'), array('', ''), $line));
			if(preg_match("/<li class='country'>/", $line))
				$this->rawr[$type][] = trim(str_replace(array("<li class='country'>", '</li>'), array('', ''), $line));
		}
		$this->p->needsave = true;
		return true;
	}
	function expandrawr() {
		if(!$this->rawr) return true;
		foreach($this->rawr as $key => $countries) {
			foreach($countries as $country) {
				$ccode = cencode($country);
				if($ccode === false)
					echo 'Unknown country: ' . $country . PHP_EOL;
				else if(!$this->range[$ccode]) {
					if(rencode($key) == 4) $comment = $key;
					$this->range[$ccode] = new Country($key, '{redlist}', $comment);
					$needsave = true;
				}
			}
		}
		return true;
	}
	function editcomments() {
		if($this->comments)
			echo "Current value of comments field: " .
				$this->comments . PHP_EOL;
		$add = $this->getline(
			"Text to be added (type 'e' to edit the entire text): "
		);
		if($add === 'e')
			$this->comments = $this->getline("New value of comments field: ");
		else if($add)
			$this->comments .= " " . $add;
	}
	function editdistribution() {
		if($this->rank !== 'species') return true;
		$this->informrange();
		makemenu(array(
			'c' => 'quit this taxon and continue',
			'q' => 'quit this taxon and stop editing ranges',
			'i' => 'get information about this taxon',
			'e' => 'edit the comments for this taxon',
			'<n> <country>' => 'set range in country <country> to <n>',
			'm <taxon>' => 'move to editing taxon <taxon>',
			), 'Editing range for taxon ' . $this->name . '. Command syntax:'
		);
		while(true) {
			$cmd = $this->getline();
			// process command
			if(strlen($cmd) > 1) {
				$arg = substr($cmd, 2);
				$cmd = $cmd[0];
			}
			switch($cmd) {
				case 'c': return true;
				case 'q': return false;
				case 'i': $this->inform(); break;
				case 'e': $this->editcomments(); break;
				case 'm':
					if($taxa[$arg])
						return $taxa[$arg]->editdistribution();
					else {
						echo 'Invalid taxon' . PHP_EOL;
						break;
					}
				case '0': case '1': case '2': case '3': case '4':
					$ccode = cencode($arg);
					if($ccode === false) {
						echo 'Invalid country' . PHP_EOL;
						break;
					}
					// source
					$osource = $source;
					$source = $this->getline(
						'Source (\'u\' to use last source used): ');
					if($source === 'q') break;
					if($source === 'u') $source = $osource;
					// comment
					$ocomment = $comment;
					$comment = $this->getline('Comment: ');
					if($comment === 'q') break;
					if($comment === 'u') $comment = $ocomment;
					// commit changes
					if(!$this->range) $this->range = array();
					if($this->range[$ccode]) {
						$this->range[$ccode]->presence = $cmd;
						$this->range[$ccode]->source = $source;
						$this->range[$ccode]->comment = $comment;
					}
					else
						$this->range[$ccode] = new Country($cmd, $source, $comment);
					$this->p->needsave = true;
					break;
				default: echo 'Invalid command' . PHP_EOL; break;
			}
		}
	}
	function my_inform() {
		$this->inform();
		$this->informrange();
		$this->informchildren();
	}
	function informrange() {
		if($this->range) {
			echo 'RANGE:' . PHP_EOL;
			foreach ($this->range as $key => $country) {
				echo cdecode($key) . ': ' . rdecode($country->presence) . PHP_EOL;
				if($country->source) echo 'Source: ' . $country->source . PHP_EOL;
				if($country->comment) echo 'Comment: ' . $country->comment . PHP_EOL;
			}
			return true;
		}
		return false;
	}
	function informchildren() {
		$children = $this->getchildren();
		if($children) {
			echo 'CHILDREN:' . PHP_EOL;
			foreach($children as $child)
				echo $child . PHP_EOL;
			return true;
		}
		return false;
	}
	public function newadd() {
		$fields = array('parent', 'rank', 'authority', 'year', 'movedgenus', 'comments', 'status');
		if($pos = strpos($this->name, ' '))
			$this->set(array('parent' => substr($this->name, 0, $pos), 'verbose' => true, 'easy' => true));
		foreach($fields as $field) {
			if($this->$field) continue;
			if($field === 'parent' and $this->detectparent()) continue;
			if($field === 'rank' and $this->detectrank()) continue;
			if($field === 'movedgenus' and $this->rank !== 'species') continue;
			self::setifneeded($paras, $field);
			if($paras[$field] === 'q') {
				$this->discardthis = true;
				return false;
			}
			else if($paras[$field])
				$this->$field = $paras[$field];
		}
	}
	private function detectparent() {
		if($this->rank === 'species') {
			$parent = substr($this->name, 0, strpos($this->name, ' '));
			if($this->p->has($parent)) {
				$this->parent = $parent;
				return true;
			}
			else
				return false;
		}
		return false;
	}
	private function detectrank() {
		if($this->rank) return true;
		if($this->setranktosisters()) return true;
		$parentrank = $this->p->get($this->parent, 'rank');
		if($parentrank === 'genus')
			$this->rank = 'species';
		else if($parentrank === 'subtribe')
			$this->rank = 'genus';
		else if(substr($this->name, -3) === 'ina')
			$this->rank = 'subtribe';
		else if(substr($this->name, -3) === 'ini')
			$this->rank = 'tribe';
		else if(substr($this->name, -4) === 'inae')
			$this->rank = 'subfamily';
		else if(substr($this->name, -4) === 'idae')
			$this->rank = 'family';
		else if(substr($this->name, -5) === 'oidea')
			$this->rank = 'superfamily';
		return $this->rank ? true : false;
	}
	private function setranktosisters() {
		$sisters = $this->p->getchildren($this->parent);
		if(!$sisters) 
			return false;
		foreach($sisters as $key => $sister) {
			$nrank = $this->p->get($sister, 'rank');
			if(!$nrank)
				return false;
			if(!isset($rank))
				$rank = $nrank;
			else if($rank !== $nrank)
				return false;
		}
		if(!isset($rank)) 
			return false;
		$this->set(array(
			'rank' => $rank, 
			'verbose' => true, 
			'easy' => true
		));
		return true;
	}
	static $rankendings = array('superfamily' => 'oidea',
		'family' => 'idae',
		'subfamily' => 'inae',
		'tribe' => 'ini',
		'subtribe' => 'ina',
	);
	private function setnametorank() {
		if(!in_array($this->rank, array('superfamily', 'family', 'subfamily', 'tribe', 'subtribe'))) return false;
		if(!preg_match('/(ina|ini|inae|idae|oidea)$/', $this->rank, $ending)) {
			echo 'Error: incorrect ending' . PHP_EOL;
			return false;
		}
		$newname = preg_replace('/' . $ending[1] . '$/u', self::$rankendings[$this->rank], $this->name);
		$this->set(array('name' => $newname, 'verbose' => true, 'easy' => true));
		return true;
	}
	/* small utilities */
	private function validrank($in = '') {
	// validate rank; may be called statically with $in set or from within the class without a parameter
		$ranks = array('class', 'order', 'suborder', 'infraorder', 'parvorder', 'superfamily', 'family', 'subfamily', 'tribe', 'subtribe', 'genus', 'species');
		if($in)
			return in_array($in, $ranks);
		else
			return in_array($this->rank, $ranks);
	}
	public function isfamilygroup() {
		return in_array($this->rank, array('superfamily', 'family', 'subfamily', 'tribe', 'subtribe'));
	}
	public function remove() {
		$this->p->remove($this->name);
	}
	public function isextant() {
		if($this->isextant) return true;
		if ($this->rank === 'species' and $this->status !== 'FO') return true;
		return false;
	}
	public function merge(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				0 => 'Taxon to be merged',
				'into' => 'Taxon that this taxon is to be merged into',
			),
			'default' => array(
				'into' => false,
			),
			'errorifempty' => array(
				0
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		while(!$this->p->has($paras['into'])) {
			$paras['into'] = $this->getline('Taxon to be merged into: ');
			if($paras['into'] === 'q') 
				return false;
		}
		$children = $this->getchildren();
		foreach($children as $child)
			$this->p->set($child, array('parent' => $paras['into']));
		$this->p->remove($this->name);
		return true;
	}
	private function sortchildren() {
		return $this->p->sortchildren($this->name);
	}
	public function populatecitation(array $paras = array()) {
	// try to find citation
		if($this->process_paras($paras, array(
			'checklist' => array('f' => 'Whether to search even though a citation is already present'),
			'default' => array('f' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($this->citation and !$paras['f'])
			return false;
		// prepare bfind query
		$authors = '/' . 	
			preg_replace(
				array('/,\s*|\s*&\s*|\s+|-|[A-Z]\./u', '/(\.\*)+/u'),
				array('.*', '.*'), 
				$this->authority
			) . 
			'/iu';
		$title = '/\b' .
			preg_replace('/\s+/u', '.*', $this->name);
		if($this->rank !== 'genus')
			$title .= '\b.*nov\b/iu';
		else
			$title .= ' nov/iu';
		global $csvlist;
		$this->p->needsave();
		echo 'Searching for possible citations for entry ' . $this->name . PHP_EOL;
		echo 'Authority: ' . $this->authority . '; year: ' . $this->year. PHP_EOL;
		if(!$this->authority or !$this->year) {
			$this->citation = 'Unknown';
			return false;
		}
		$cites = array_merge(
			$csvlist->bfind(array(
				'authors' => $authors,
				'year' => $this->year,
				'print' => false,
			)),
			$csvlist->bfind(array(
				'name' => $title,
				'print' => false,
			))
		);
		if(!$cites) {
			echo 'Nothing found' . PHP_EOL;
			$this->citation = 'Unknown';
			return true;
		}
		foreach($cites as $cite) {
			$taxon = $this;
			$response = $this->menu(array(
				'head' => 'Is this citation correct?' . PHP_EOL . 
					$cite->name . PHP_EOL . 
					$cite,
				'options' => array(
					'y' => 'This citation is correct',
					'n' => 'This citation is not correct',
					's' => 'This citation is not correct, and stop listing others',
					'ic' => 'Give more information about the citation',
					'it' => 'Give more information about the taxon',
					'o' => 'Open the citation file',
				),
				'process' => array(
					'ic' => function() use($cite) {
						$cite->inform();
					},
					'it' => function() use($taxon) {
						$taxon->inform();
					},
					'o' => function() use($cite) {
						$cite->openf();
					}
				),
			));
			switch($response) {
				case 'y': 
					$this->citation = $cite->name;
					return true;
				case 'n': break;
				case 's': break 2;
			}
		}
		$this->citation = 'Unknown';
		return true;
	}
}
?>
