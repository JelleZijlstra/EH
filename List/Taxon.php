<?php
class Taxon extends CsvListEntry {
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
	static $parentlist = 'taxonlist';
	protected static $arrays_to_check = array('ids', 'endemic', 'misc');
	public $ngen;
	public $nspec;
	public $isextant;
	public $citation; // citation to the original description
	// flag to $this->p->addEntry() that this entry should not be added.
	// Should get rid of it; perhaps instead just set $this->name to NULL, or
	// use a StopException
	public $discardthis;
	protected static $Taxon_commands = array(
		'populatecitation' => array('name' => 'populatecitation',
			'desc' => 'Attempt to populate the citation field',
			'aka' => 'addcite'),
		'getChildren' => array('name' => 'getChildren',
			'desc' => 'Return an array of a taxon\'s children'),
	);
	function __construct($in, $code, &$parent) {
	// $in: input data (array or string)
	// $code: kind of Taxon to make
		$this->p =& $parent;
		switch($code) {
			case 'f': // loading from file
				if(!is_array($in)) {
					throw new EHException(
						"Input to Article constructor is not an array"
					);
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
				$this->newadd($in);
				break;
			default:
				throw new EHException("Invalid input to Taxon constructor");
		}
	}
	public function toArray() {
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
		if(preg_match("/^A new species \([^)]+\)\{[^}]+\}\.?$/u", 
			$this->comments) and $this->citation) {
			$this->comments = 'A new species.';
		}
		if(preg_match(
			'/^A new genus,? split from <i>[^<]+<\/i> \([^)]+\)\{[^}]+\}\.?$/u', 
			$this->comments) and $this->citation) {
			$this->comments = preg_replace(
				'/^A new genus,? split from (<i>[^<]+<\/i>).*$/u',
				'A new genus split from $1.',
				$this->comments
			);
		}
		/* WARNINGS */
		if(!$this->p->has($this->parent))
			$this->warn('Non-existent taxon', 'parent');
		if($this->rank === 'species' and !$this->status)
			$this->warn('No content for species', 'status');
		if($this->citation) {
			$regex = '/' . preg_quote($this->citation) . '/u';
			if(preg_match($regex, $this->comments)) {
				$this->warn('Original citation is cited', 'comments');
			}
		}
		return true;
	}
	public function set(array $paras) {
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
						$this->p->addEntry($this);
						$this->p->remove(array($oldname));
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
	public function children() {
		if(!isset($this->p->par[$this->name]))
			return NULL;
		else
			return $this->p->par[$this->name];
	}
	public function getChildren(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array( /* No paras */ ),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$children = $this->children();
		if($children === NULL) {
			return array();
		} else {
			return array_keys($children);
		}
	}
	private function call_children($func, array $paras) {
		if(!isset($this->p->par[$this->name]))
			return;
		$children = $this->p->par[$this->name];
		foreach($children as $child) {
			$paras[] = $child;
		}
		$this->p->$func($paras);
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
		if($children = $this->children()) foreach($children as $child)
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
		$out = PHP_EOL . $pre . ucfirst($this->rank) . ': ' . $this->name . PHP_EOL;
		$out .= $pre . 'Authority: ' . $this->constructauthority() . PHP_EOL;
		if($this->originalref)
			$out .= $pre . 'Original reference: ' . $this->parse('originalref', 'simple') . PHP_EOL;
		if($this->comments)
			$out .= $pre . 'Comments: ' . $this->parse('comments', 'refend');
		$out .= PHP_EOL;
		if($children = $this->children()) foreach($children as $child) {
			$out .= $this->p->text($child);
		}
		return $out;
	}
	public function wiki(array $paras = array()) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'taxon' => 'Taxon associated with the file we are writing to'
			),
			'errorifempty' => array('taxon'),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
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
				if($this->citation and $this->citation !== 'Unknown') {
					$csvlist = ArticleList::singleton();
					$tmp .= ':::<small>Original description: ';
					$csvlist->verbosecite = false;
					$tmp .= $csvlist->cite($this->citation);
					$tmp .= '</small>' . PHP_EOL;
				}
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
				$children = $this->children();
				if(!$children) break;
				$this->sortchildren();
				foreach($children as $child) {
					if($this->p->get($child)->status !== 'FO') {
						$this->nspec++;
						$this->isextant = true;
					}
				}
				break;
			case "species":
				$this->movedgenus = $this->movedgenus ? 1 : '';
				break;
			default:
				$children = $this->children();
				if(!$children) break;
				// don't sort the orders
				if($this->rank !== 'class') $this->sortchildren();
				foreach($children as $child) {
					$this->p->completedata($child);
					if($this->p->get($child)->isextant === true) {
						$this->ngen += $this->p->get($child)->ngen;
						$this->nspec += $this->p->get($child)->nspec;
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
		$this->p->needsave();
		return true;
	}
	function expandrawr() {
		if(!$this->rawr)
			return true;
		foreach($this->rawr as $key => $countries) {
			foreach($countries as $country) {
				$ccode = cencode($country);
				if($ccode === false)
					echo 'Unknown country: ' . $country . PHP_EOL;
				else if(!$this->range[$ccode]) {
					if(rencode($key) === 4)
						$comment = $key;
					$this->range[$ccode] = new Country($key, '{redlist}', $comment);
					$this->p->needsave();
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
					$this->p->needsave();
					break;
				default: echo 'Invalid command' . PHP_EOL; break;
			}
		}
	}
	public function inform(array $paras = array()) {
		parent::inform($paras);
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
		$children = $this->children();
		if($children) {
			echo 'CHILDREN:' . PHP_EOL;
			foreach($children as $child)
				echo $child . PHP_EOL;
			return true;
		}
		return false;
	}
	public function newadd(array $paras) {
		// fields to fill
		$fields = array(
			'name', 'parent', 'rank', 'authority', 'year', 'movedgenus', 
			'comments', 'status', 'citation',
		);
		// set things for species
		if(isset($paras['name'])) {
			$pos = strpos($paras['name'], ' ');
			// if there is a space, this is a species
			if($pos !== false) {
				$paras['rank'] = 'species';
				$paras['parent'] = substr($paras['name'], 0, $pos);
			}
		}
		// detect rank
		if(!isset($paras['rank'])) {
			$rank = $this->detectrank($paras);
			if($rank !== false) {
				$paras['rank'] = $rank;
			}
		}
		// movedgenus == 0 if this is not a species
		if(!isset($paras['movedgenus']) and isset($paras['rank']) 
			and $paras['rank'] !== 'species') {
			$paras['movedgenus'] = 0;
		}
		// so that we kill this if the user interrupts during pp call
		$this->discardthis = true;
		// ask user
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checkfunc' => function($in) use($fields) {
				return in_array($in, $fields, true);
			},
			'askifempty' => $fields,
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$this->discardthis = false;
		$paras['verbose'] = true;
		$paras['easy'] = true;
		$this->set($paras);
		return true;
	}
	private function detectrank(array $paras) {
		if(isset($paras['parent'])) {
			$parent = $paras['parent'];
			$sisrank = $this->setranktosisters($parent);
			if($sisrank !== false) {
				return $sisrank;
			}
			$parentrank = $this->p->get($parent)->rank;
			if($parentrank === 'genus') {
				return 'species';
			} else if($parentrank === 'subtribe') {
				return 'genus';
			}
		}
		if(isset($paras['name'])) {
			$name = $paras['name'];
			if(substr($name, -3) === 'ina') {
				return 'subtribe';
			} else if(substr($name, -3) === 'ini') {
				return 'tribe';
			} else if(substr($name, -4) === 'inae') {
				return 'subfamily';
			} else if(substr($name, -4) === 'idae') {
				return 'family';
			} else if(substr($name, -5) === 'oidea') {
				return 'superfamily';
			}
		}
		return false;
	}
	private function setranktosisters($parent = NULL) {
		if($parent === NULL) {
			$parent = $this->parent;
		}
		$sisters = $this->p->children($parent);
		if(!$sisters) {
			return false;
		}
		foreach($sisters as $key => $sister) {
			$nrank = $this->p->get($sister)->rank;
			if(!$nrank) {
				return false;
			}
			if(!isset($rank)) {
				$rank = $nrank;
			} else if($rank !== $nrank) {
				return false;
			}
		}
		if(!isset($rank))
			return false;
		return $rank;
	}
	static $rankendings = array(
		'superfamily' => 'oidea',
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
	// Wrapper function, do we really need it?
		$this->p->remove(array($this->name));
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
				'into' => 'Taxon that this taxon is to be merged into',
			),
			'askifempty' => array('into'),
			'checkparas' => array(
				'into' => function($in) {
					return TaxonList::singleton()->has($in);
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$children = $this->children();
		foreach($children as $child)
			$this->p->set(array(0 => $child, 'parent' => $paras['into']));
		$this->p->remove(array($this->name));
		return true;
	}
	private function sortchildren() {
		return $this->p->sortchildren($this->name);
	}
	public function populatecitation(array $paras = array()) {
	// try to find citation
		if($this->process_paras($paras, array(
			'checklist' => array(
				'f' => 'Whether to search even though a citation is already present',
				'c' => 'Whether to search even though an "unknown" citation is already present'
			),
			'default' => array('f' => false, 'c' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		if($paras['c']) {
			if($this->citation and $this->citation !== 'Unknown')
				return false;
		}
		else if(!$paras['f']) {
			if($this->citation)
				return false;
		}
		// prepare bfind query
		$authors = '/' .
			preg_replace(
				array('/,\s*|\s*&\s*|\s+|-|[A-Z]\./u', '/(\.\*)+/u'),
				array('.*', '.*'),
				$this->authority
			) .
			'/iu';
		$title = '/\b' .
			preg_replace('/\s+/u', '.*\b', $this->name);
		if($this->rank !== 'genus')
			$title .= '(?! [a-z]+ nov)\b.*\bnov\b/u';
		else
			$title .= ' \bnov\b/u';
		$this->p->needsave();
		echo 'Searching for possible citations for entry ' . $this->name . PHP_EOL;
		echo 'Authority: ' . $this->authority . '; year: ' . $this->year. PHP_EOL;
		if(!$this->authority or !$this->year) {
			$this->citation = 'Unknown';
			return false;
		}
		$csvlist = ArticleList::singleton();
		$cites = array_merge(
			$csvlist->bfind(array(
				'authors' => $authors,
				'year' => $this->year,
				'quiet' => true,
			)),
			$csvlist->bfind(array(
				'name' => $title,
				'quiet' => true,
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
						return true;
					},
					'it' => function() use($taxon) {
						$taxon->inform();
						return true;
					},
					'o' => function() use($cite) {
						$cite->openf();
						return true;
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
