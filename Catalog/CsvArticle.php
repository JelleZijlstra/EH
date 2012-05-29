<?php
// methods that should not get redirects resolved by ContainerList
ContainerList::$resolve_redirect_exclude[] = array('CsvArticle', 'isredirect');

class CsvArticle extends CsvListEntry implements ArticleInterface {
	use CommonArticle;
	
	// references to a Folder object in Article
	public $folder; //folder (NOFILE when "file" is not a file)
	public $sfolder; //subfolder
	public $ssfolder; //sub-subfolder
	
	// timestamp in Article
	public $addmonth; //month added to catalog
	public $addday; //day added to catalog
	public $addyear; //year added to catalog
	
	// part of identifiers in Article
	public $url; //url where available
	public $doi; //DOI
	
	// part of publisher in Article
	public $location; //geographical location published
	protected function publisher() {
		return $this->publisher;
	}
	protected function location() {
		return $this->location;
	}
	protected function journal() {
		return $this->journal;
	}
	protected function doi() {
		if(strlen($this->doi) > 0) {
			return $this->doi;
		} else {
			return false;
		}
	}
	protected function jstor() {
		if(strlen($this->jstor) > 0) {
			return $this->jstor;
		} else {
			return false;
		}
	}
	protected function hdl() {
		if(strlen($this->hdl) > 0) {
			return $this->hdl;
		} else {
			return false;
		}
	}
	protected function _getIdentifier($name) {
		$out = $this->$name;
		if(strlen($out) > 0) {
			return $out;
		} else {
			return false;
		}
	}
	protected function name() {
		return $this->name;
	}
	
	public $ids; //array of properties for various less-common identifiers
	public $bools; // array of boolean flags
	static $n_ids = array('isbn', 'eurobats', 'hdl', 'jstor', 'pmid', 'edition', 'issn', 'pmc'); // names of identifiers supported
	static $n_bools = array('parturl', 'fullissue'); // variables (mostly boolean) supported
	protected static $arrays_to_check = array('ids', 'bools');
	protected static $CsvArticle_commands;
	/* OBJECT CONSTRUCTION AND BASIC OPERATIONS */
	public function __construct($in, $code, &$parent) {
	// $in: input data (array or string)
	// $code: kind of Article to make
		$this->p =& $parent;
		switch($code) {
			case 'f': // loading from file
				if(!is_array($in)) {
					throw new EHException(
						"Input to Article constructor is not an array"
					);
				}
				$this->name = $in[6];
				$this->folder = $in[3];
				$this->sfolder = $in[4];
				$this->ssfolder = $in[5];
				$this->addmonth = $in[0];
				$this->addday = $in[1];
				$this->addyear = $in[2];
				$this->authors = $in[7];
				$this->year = $in[8];
				$this->title = $in[9];
				$this->journal = $in[10];
				$this->series = $in[11];
				$this->volume = $in[12];
				$this->issue = $in[13];
				$this->start_page = $in[14];
				$this->end_page = $in[15];
				$this->url = $in[16];
				$this->doi = $in[17];
				$this->type = (int) $in[18];
				$this->publisher = $in[20];
				$this->location = $in[21];
				$this->pages = $in[22];
				if($in[24]) $this->ids = json_decode($in[24], true);
				if($in[26]) $this->bools = json_decode($in[26], true);
				if(isset($in[27])) $this->parent = $in[27];
				if(isset($in[28])) $this->misc_data = $in[28];
				return;
			case 'r': // make new redirect
				if(!is_array($in)) {
					throw new EHException(
						"Input to Article constructor is not an array"
					);
				}
				$this->name = $in[0];
				$this->type = self::REDIRECT;
				$this->parent = $in[1];
				break;
			case 'n': // new NOFILE entry
				// revise so it can somewhat take into account ListEntry::add()
				$this->folder = 'NOFILE';
				if(is_string($in)) {
					$this->name = $in;
				} elseif(is_array($in)) {
					// assume that ListEntry::add() gave us this input, and it makes sense
					// Ultimately, Article's entire "adding" system should be revised.
					foreach($in as $key => $value) {
						$this->$key = $value;
					}
				} else {
					throw new EHException(
						"Input to Article constructor is not a string"
					);
				}
				break;
			case 'b': case 'l': // add new entry from basic information. 'l' if calling Article::add() is not desired
				if(!is_array($in)) {
					throw new EHException(
						"Input to Article constructor is not an array"
					);
				}
				$this->name = $in[0];
				if(is_array($in[1])) {
					$in[1] = array_pad($in[1], 3, '');
					$this->folder = $in[1][0];
					$this->sfolder = $in[1][1];
					$this->ssfolder = $in[1][2];
				} else {
					$this->folder = $in[1];
					$this->sfolder = $in[2];
					$this->ssfolder = $in[3];
				}
				if($code === 'l') {
					return;
				} else {
					break;
				}
			case 'e': // empty Article
				return;
			default:
				throw new EHException("Invalid code for Article constructor");
		}
		// add additional data
		$this->add();
	}
	private function needSave() {
		$this->p->needsave();
	}
	public static function init() {
		self::$CsvArticle_commands = self::$Article_commands;
	}
	/*
	 * Factory methods.
	 */
	public static function makeEmpty(&$parent) {
		return new self(NULL, 'e', $parent);
	}
	public static function makeNewNofile(/* string */ $name, ContainerList $parent) {
		$obj = new self(NULL, 'e', $parent);
		$obj->folder = 'NOFILE';
		$obj->name = $name;
		$obj->add();
		$parent->add($obj, array('isnew' => true));
		return $obj;
	}
	public static function makeNewRedirect($name, $target, ContainerList $parent) {
		$obj = new self(NULL, 'e', $parent);
		$obj->type = self::REDIRECT;
		$obj->folder = $target;
		$obj->name = $name;
		$obj->setCurrentDate();
		$parent->addEntry($obj, array('isnew' => true));
		return $obj;
	}
	public function toArray() {
		$out = array();
		$out[] = $this->addmonth;
		$out[] = $this->addday;
		$out[] = $this->addyear;
		$out[] = $this->folder;
		$out[] = $this->sfolder;
		$out[] = $this->ssfolder;
		$out[] = $this->name;
		$out[] = $this->authors;
		$out[] = $this->year;
		$out[] = $this->title;
		$out[] = $this->journal;
		$out[] = $this->series;
		$out[] = $this->volume;
		$out[] = $this->issue;
		$out[] = $this->start_page;
		$out[] = $this->end_page;
		$out[] = $this->url;
		$out[] = $this->doi;
		$out[] = $this->type;
		$out[] = '';
		$out[] = $this->publisher;
		$out[] = $this->location;
		$out[] = $this->pages;
		$out[] = '';
		$out[] = $this->getarray('ids');
		$out[] = '';
		$out[] = $this->getarray('bools');
		$out[] = $this->parent;
		$out[] = $this->misc_data;
		return $out;
	}
	private function _path(array $paras) {
		// if there is no folder, just return filename and hope for the best
		if($this->folder === NULL) {
			$out = $this->name;
		} else {
			if($paras['fullpath']) {
				$out = LIBRARY . '/';
			} else {
				$out = '';
			}
			$out .= $this->folder;
			if($this->sfolder) {
				$out .= "/" . $this->sfolder;
				if($this->ssfolder)
					$out .= "/" . $this->ssfolder;
			}
			if(!$paras['folder'])
				$out .= "/" . $this->name;
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
	/* FORMATTING */
	public function format(array $paras = array()) {
	// This function does various kinds of cleanups and tweaks. It's a mess,
	// though, and should be organized better.
		/*
		 * Fix old style
		 */
		$this->setType();
		if(substr($this->folder, 0, 4) === 'SEE ') {
			$this->parent = $this->resolve_redirect();
			$this->folder = 'NOFILE';
		}
		if(isset($this->title[0]) && ($this->title[0] === '/')) {
			$tmp = explode('/', $this->title);
			$this->title = $tmp[1];
			$this->parent = $tmp[2];
		}
		if(preg_match('/^([A-Za-z]+) thesis, (.*)$/u', $this->publisher, $matches)) {
			$this->series = $matches[1];
			$this->publisher = $matches[2];
		}
		
		/*
		 * completion of partial citations
		 */
		// expand journals
		$this->expandjournal();
		// automatically detect title in MS papers
		if(!$this->title && preg_match("/^MS /", $this->name))
			$this->title = preg_replace("/^MS | n|.pdf$/", "", $this->name);
		/*
		 * conversion
		 */
		if($this->isredirect()) {
			// no other data for redirects
			$redirect_remove = array('sfolder', 'ssfolder', 'authors', 'year', 
				'title', 'journal', 'volume', 'series', 'issue', 'start_page', 
				'end_page', 'pages', 'ids', 'doi', 'url', 'location', 'bools'
			);
			foreach($redirect_remove as $key)
				$this->$key = NULL;
			$target = $this->resolve_redirect();
			if(!$this->p->has($target))
				$this->warn('invalid redirect target', 'folder');
			else if($this->p->isredirect($target))
				$this->parent = $this->p->resolve_redirect($target);
		}
		if($this->issupplement()) {
			$supplement_remove = array('authors', 'year', 'journal', 'volume', 
				'series', 'issue', 'start_page', 'end_page', 'pages', 'ids', 
				'doi', 'url', 'location', 'bools'
			);
			foreach($supplement_remove as $key)
				$this->$key = NULL;
			$target = $this->supp_getbasic();
			// resolve redirect
			if($this->p->isredirect($target))
				$this->parent = $this->p->resolve_redirect($target);
		}
		foreach(array('parturl') as $field) {
			if($this->$field) {
				$this->$field = 1;
			} else {
				unset($this->$field);
			}
		}
		// replace with DOI
		if($this->jstor) {
			$this->doi = '10.2307/' . $this->jstor;
			$this->jstor = NULL;
		}
		// this indicates it's in press
		if($this->start_page === 'no') {
			$this->start_page = 'in press';
			$this->end_page = NULL;
		}
		// correct dashes (also prevent OpenOffice from bad things)
		$this->volume = str_replace('-', "–", $this->volume);
		$this->issue = preg_replace("/[-_]/u", "–", $this->issue);
		$this->year = str_replace('-', "–", $this->year);
		// all-uppercase author names corrected (disabled because of "IUCN")
		$this->authors = preg_replace_callback(
			"/(?<=^|\s)[A-Z]+(?=[,\s])/u",
			function($uppercase) {
				return ucfirst(mb_strtolower($uppercase[0], "UTF-8"));
			},
			$this->authors
		);
		// typo I often make (> for .), and stuff that just happens
		$this->authors = str_replace(array('>', ';;'), array('.', ';'), $this->authors);
		// Jr. problems
		$this->authors = preg_replace('/; Jr\.(?=;|$)/', ', Jr.', $this->authors);
		// cap after hyphen
		$this->authors = preg_replace_callback(
			"/(?<=-)(\w)(?!\.)/u",
			function($matches) {
				return mb_strtoupper($matches[0], 'UTF-8');
			},
			$this->authors
		);
		// some journals place the volume in "issue"
		if($this->issue && !$this->volume) {
			$this->volume = $this->issue;
			$this->issue = NULL;
		}
		// clean up titles a little: space before and after parentheses, but not in e.g. "origin(s)" (hidden as this is buggy)
		//$this->title = trim(preg_replace(array("/(?<!\s|^)\((?!s\))/u", "/\)(?!\s|$|,|\.|:|<)/u"), array(" (", ") "), $this->title));
		//$this->title = preg_replace("/(?<!\s|\(|-|^)([A-Z])/u", " $1", $this->title);
		if(substr($this->url, 0, 28) === 'http://www.jstor.org/stable/') {
		// put JSTOR in
			$this->jstor = substr($this->url, 28);
			$this->url = NULL;
		}
		if(substr($this->url, 0, 22) === 'http://hdl.handle.net/') {
		// and HDL
			$this->hdl = substr($this->url, 22);
			$this->url = NULL;
		}
		if(preg_match("/^http:\/\/dx\.doi\.org\//", $this->url)) {
		// put DOI in and remove redundant URL
			$this->doi = preg_replace("/^http:\/\/dx\.doi\.org\//", "", $this->url);
			$this->url = NULL;
		}
		if(preg_match("/^http:\/\/www\.bioone\.org\/doi\/(full|abs|pdf)\/(.*)$/", $this->url, $doim)) {
			$this->doi = $doim[2];
			$this->url = NULL;
		}
		if(preg_match("/^http:\/\/onlinelibrary\.wiley\.com\/doi\/(.*?)\/(abs|full|pdf|abstract)$/", $this->url, $doim)) {
			$this->doi = $doim[1];
			$this->url = NULL;
		}
		if(preg_match('/^http:\/\/(digitallibrary\.amnh\.org\/dspace|deepblue\.lib\.umich\.edu)\/handle\/(.*)$/', $this->url, $hdlm)) {
			$newhdl = $hdlm[2];
			if(!$this->hdl) {
				$this->hdl = $newhdl;
			}
			else if($this->hdl !== $newhdl) {
				echo 'Recorded HDL for file ' . $this->name . ': ' . $this->hdl . '. HDL in URL: ' . $newhdl . PHP_EOL;
			}
			$this->url = NULL;
		}
		// fix DOI formatting
		$this->doi = rawurldecode($this->doi);
		$this->title = preg_replace(
		// remove final period and curly quotes, italicize cyt b, other stuff
			array(
				'/(?<!\\\\)\.$|^\s+|\s+$|(?<=^<i>)\s+|<i><\/i>|☆/u',
				'/<\/i>\s+<i>|\s+/u',
				'/^" |(?<= )" |&quot;/u',
				'/([,:();]+)<\/i>/u',
				'/<i>([,:();]+)/u',
				'/[`‘’]/u',
				'/(?<=\bcytochrome[- ])b\b/u',
				'/ - /u',
				'/(?<=<|<\/)I(?=>)/u',
			), array(
				'',
				' ',
				'"',
				'</i>$1',
				'$1<i>',
				"'",
				'<i>b</i>',
				' – ',
				'i',
			), $this->title);
		if(strpos($this->publisher, ': ') !== false) {
		// remove final period and curly quotes
			$tmp = explode(': ', $this->publisher);
			$this->publisher = $tmp[1];
			$this->location = $tmp[0];
		}
		if(preg_match('/^(PhD|MSc) thesis, /', $this->journal)) {
		// "PhD thesis" should be in "publisher" field
			$this->publisher = $this->journal;
			$this->journal = NULL;
		}
		// redundant stuff for books
		if($this->parent) {
			$obj = $this->getEnclosing();
			if($this->publisher) {
				if(!$obj->publisher) {
					$obj->publisher = $this->publisher;
					$this->publisher = '';
				} else {
					if($this->publisher !== $obj->publisher) {
						$this->warn('Publisher for enclosing ("' .
							$obj->publisher . '") is different from publisher',
							'publisher');
					} else {
						$this->publisher = '';
					}
				}
			}
			if($this->location) {
				if(!$obj->location) {
					$obj->location = $this->location;
					$this->location = '';
				} else {
					if($this->location !== $obj->location) {
						$this->warn('Location for enclosing ("' .
							$obj->location . '") is different from location',
							'location');
					} else {
						$this->location = '';
					}
				}
			}
			if($this->pages) {
				if(!$obj->pages) {
					$obj->pages = $this->pages;
					$this->pages = '';
				} else {
					if($this->pages !== $obj->pages) {
						$this->warn('Pages for enclosing ("' .
							$obj->pages . '") is different from pages',
							'pages');
					} else {
						$this->pages = '';
					}
				}
			}
		}
		/*
		 * do things that are necessary for all properties
		 */
		$this->formatallprops();
		/*
		 * check possible errors
		 */
		if(!$this->folder)
			$this->warn('no content', 'folder');
		if(!$this->isor('redirect', 'fullissue') && !$this->title) {
			var_dump($this->type);
			var_dump($this->isredirect());
			$this->warn('no content', 'title');
		}
		if(!$this->isor('redirect', 'fullissue', 'supplement', 'erratum') && !$this->authors)
			$this->warn('no content', 'authors');
		if(!$this->isor('redirect', 'inpress') && !$this->volume && $this->journal)
			$this->warn('no content', 'volume');
		// odd stuff in authors
		if(strpos($this->authors, ';;') !== false)
			$this->warn('double semicolon', 'authors');
		if(preg_match('/; ([JS]r\.|[Ii]+)(;|$)/', $this->authors))
			$this->warn('stray junior', 'authors');
		// DOI
		if(strlen($this->doi) && !preg_match('/^10\./u', $this->doi)) {
			$this->warn('invalid content', 'doi');
		}
		// content of "year" is tightly constrained
		if(!$this->isor('redirect', 'supplement', 'erratum', 'inpress') and !preg_match('/\d{4}|\d{4}–\d{4}|undated/', $this->year))
			$this->warn('invalid content', 'year');
		// DOI titles tend to produce this kind of mess
		if(preg_match("/[A-Z] [A-Z] [A-Z]/", $this->title))
			$this->warn('spaced caps', 'title');
		// periods may indicate abbreviations, which we don't want
		// allow escaping with \
		if(preg_match("/(?<!\\\\)\./", $this->journal))
			$this->warn('period', 'journal');
		// buggy AMNH code tends to cause this
		if(preg_match("/\((?!ed\.\)|eds\.\))/", $this->authors))
			$this->warn('parenthesis', 'authors');
		// bug in previous code
		if($this->authors and ($this->authors === $this->title))
			$this->warn('authors equal with title', 'authors');
		// buggy Geodiversitas and AMNH code tends to cause this
		// OpenOffice weirdness
		if(preg_match("/\//", $this->issue))
			$this->warn('slash', 'issue');
		if(preg_match("/\//", $this->volume))
			$this->warn('slash', 'volume');
		if(strpos($this->volume, ':') !== false)
			$this->warn('colon', 'volume');
		if($this->start_page === 'no')
			$this->warn('text "no"', 'start_page');
		// AMNH journals need HDL thingy
		if($this->isamnh() && !$this->hdl)
			$this->warn('no HDL for AMNH title', 'journal');
		if(!$this->isor('inpress', 'nopagenumberjournal', 'fullissue') and $this->journal and !$this->start_page)
			$this->warn('no content in "start_page" for journal article', 'journal');
		if($this->parent and !$this->p->has($this->parent)) {
			$this->warn('invalid enclosing article', 'parent');
		}
		$this->needSave();
		return true;
	}
	private function formatallprops() {
		// TODO: cap after apostrophes (D'elia)
		// get rid of fancy apostrophes and quotes
		foreach($this as $key => $field) {
			if(is_string($field)) {
				$this->$key = preg_replace(array("/([`’‘]|&apos;)/u", '/[“”]/u'), array("'", '"'), $this->$key);
				if(($key !== 'pdfcontent') and preg_match("/(\n|\r)/", $field))
					$this->warn('line break', $key);
				if(strpos($field, '??') !== false)
					$this->warn('double question mark', $key);
			}
		}
	}
	private function expandjournal($in = '') {
		$i = $in ?: $this->journal;
		switch($i) {
			case 'AC': $o = 'Acta Chiropterologica'; break;
			case 'AGH': case 'Acta Geologica Hispanica': $o = 'Acta Geológica Hispánica'; break;
			case 'AJPA': $o = 'American Journal of Physical Anthropology'; break;
			case 'Alcheringa: An Australasian Journal of Palaeontology': $o = 'Alcheringa'; break;
			case 'AMN': case 'American Museum novitates': $o = 'American Museum Novitates'; break;
			case 'AMNR': case 'Arquivos do Museu Nacional, Rio de Janeiro': $o = 'Arquivos do Museu Nacional'; break;
			case 'ANMW': $o = 'Annales des Naturhistorischen Museums in Wien'; break;
			case 'APP': $o = 'Acta Palaeontologica Polonica'; break;
			case 'APR': $o = 'Acta Palaeontologica Romaniae'; break;
			// note that this was apparently 'Acta Societatis Zoologicae Bohemoslovenicae' before 1989 and 'ASZ Bohemoslovacae' 1990–1992
			case 'ASZB': $o = 'Acta Societatis Zoologicae Bohemicae'; break;
			case 'AZM': case 'Acta Zoologica Mexicana': $o = 'Acta Zoológica Mexicana'; break;
			case 'AZC': case 'Acta zoologica cracoviensia': $o = 'Acta Zoologica Cracoviensia'; break;
			case 'AZF': case 'Annales Zoologicae Fennici': $o = 'Annales Zoologici Fennici'; break;
			case 'BAMNH': $o = 'Bulletin of the American Museum of Natural History'; break;
			case 'BFMNH': $o = 'Bulletin of the Florida Museum of Natural History'; break;
			case 'BKBIN': $o = "Bulletin de l'Institut royal des sciences naturales de Belgique / Bulletin van het Koninklijk Belgisch Instituut voor Natuurwetenschappen"; break;
			case 'BMNHN': $o = "Bulletin du Muséum national d'histoire naturelle"; break;
			case 'BSHNB': case 'Boll. Soco Hist. Nat. Balears': $o = "Bolleti de la Societat d'Historia Natural de les Balears"; break;
			case 'BZB': case 'Bonner zoologische beiträge': $o = 'Bonner zoologische Beiträge'; break;
			case 'BZN': $o = 'Bulletin of Zoological Nomenclature'; break;
			case 'BZP': $o = 'Beiträge zur Paläontologie'; break;
			case 'CFS': $o = 'Courier Forschungsinstitut Senckenberg'; break;
			case 'CJS': $o = 'Caribbean Journal of Science'; break;
			case 'CJZ': $o = 'Canadian Journal of Zoology'; break;
			case 'CL': $o = 'Check List'; break;
			case 'CMPUM': case 'Contributions from the Museum of Paleontology, The University of Michigan': $o = 'Contributions from the Museum of Paleontology, University of Michigan'; break;
			case 'CN': $o = 'Chiroptera Neotropical'; break;
			case 'Courier Forschungsinstitut Senckenberg': $o = 'Courier Forschungs-Institut Senckenberg'; break;
			case 'CPMHNM': case 'Comunicaciones Paleontologicas del Museo de Historia Natural de Montevideo': $o = 'Comunicaciones Paleontológicas del Museo de Historia Natural de Montevideo'; break;
			case 'Comptes Rendus de l\'Académie des Sciences de Paris, Earth and Planetary Sciences': case 'Comptes Rendus de l\'Académie des Sciences de Paris, Sciences de la Terre et des planètes': $o = 'Comptes Rendus de l\'Académie des Sciences - Series IIA - Earth and Planetary Science'; break;
			case 'CP': case 'COL-PA': $o = 'Coloquios de Paleontología'; break;
			case 'EA': case 'Evolutionary Anthropology': $o = 'Evolutionary Anthropology: Issues, News, and Reviews'; break;
			case 'EG': case 'Estudios Geologicos': $o = 'Estudios Geológicos'; break;
			case 'HIJM': $o = 'Hystrix Italian Journal of Mammalogy'; break;
			case 'HJG': $o = 'Hellenic Journal of Geosciences'; break;
			case 'Iheringia': case 'Iheringia Zoología': $o = 'Iheringia, Série Zoologia'; break;
			case 'JM': $o = 'Journal of Mammalogy'; break;
			case 'JHE': $o = 'Journal of Human Evolution'; break;
			case 'JME': $o = 'Journal of Mammalian Evolution'; break;
			case 'JOZ': case 'Journal of Zoology': $o = 'Journal of Zoology, London'; break;
			case 'JP': case 'The Journal of Parasitology': $o = 'Journal of Parasitology'; break;
			case 'JPSI': $o = 'Journal of the Palaeontological Society of India'; break;
			case 'JVP': $o = 'Journal of Vertebrate Paleontology'; break;
			case 'JSP': $o = 'Journal of Systematic Palaeontology'; break;
			case 'JSZ': case 'JZS': $o = 'Journal of Zoological Systematics and Evolutionary Research'; break;
			case 'JVMS': case 'Journal of Veterinary and Medical Sciences': $o = 'Journal of Veterinary Medical Science'; break;
			case 'MB': case 'Mammalian Biology - Zeitschrift fur Saugetierkunde': case 'Mammalian Biology - Zeitschrift fur Säugetierkunde': case 'Mammalian Biology - Zeitschrift für Säugetierkunde': $o = 'Mammalian Biology'; break;
			case 'MBE': $o = 'Molecular Biology and Evolution'; break;
			case 'MIOC': case 'Mem. Inst. Oswaldo Cruz': $o = 'Memórias do Instituto Oswaldo Cruz'; break;
			case 'MN': $o = 'Mastozoología Neotropical'; break;
			case 'MNSM': case 'Memoirs of the National Science Museum, Tokyo': $o = 'Memoirs of the National Science Museum'; break;
			case 'MBSPHG': case 'Mitteilungen der Bayerischen Staatssammlung für Paläontologie und Historische Geologie': $o = 'Mitteilungen der Bayerischen Staatssammlung für Paläontologie und historische Geologie'; break;
			case 'MPE': $o = 'Molecular Phylogenetics and Evolution'; break;
			case 'MPMZUM': $o = 'Miscellaneous Publications, Museum of Zoology, University of Michigan'; break;
			case 'MS': $o = 'Mammalian Species'; break;
			case 'MTEPHE': case 'Mém. Trav. E.P.H.E., Inst. Montpellier': $o = "Mémoires et Travaux de l'École Pratique des Hautes Études, Institut de Montpellier"; break;
			case 'NJGPA': case 'Neues Jahrbuch für Geologie und Paläontologie - Abhandlungen': $o = 'Neues Jahrbuch für Geologie und Paläontologie, Abhandlungen'; break;
			case 'NJGPMH': case 'NJGPM': case 'Neues Jahrbuch für Geologie und Paläontologie - Monatshefte': $o = 'Neues Jahrbuch für Geologie und Paläontologie, Monatshefte'; break;
			case 'OPMZUM': $o = 'Occasional Papers of the Museum of Zoology, University of Michigan'; break;
			case 'OPMZLSU': $o = 'Occasional Papers of the Museum of Zoology, Louisiana State University'; break;
			case 'OPTTU': case 'OPMTTU': case 'Occasional Papers, The Museum, Texas Tech University': $o = 'Occasional Papers, Museum of Texas Tech University'; break;
			case 'PAZ': case 'Papéis Avulsos de Zoologia, Museu de Zoologia da Universidade de São Paulo': $o = 'Papéis Avulsos de Zoologia'; break;
			case 'Paleobios': $o = 'PaleoBios'; break;
			case 'PBSW': $o = 'Proceedings of the Biological Society of Washington'; break;
			case 'PE': $o = 'Palaeontologia Electronica'; break;
			case 'PIE': $o = 'Paleontologia i Evolució'; break;
			case 'PLB': case 'PLOS Biology': $o = 'PLoS Biology'; break;
			case 'PLO': case 'PLoS one': case 'PloS ONE': $o = 'PLoS ONE'; break;
			case 'PLSNSW': $o = 'Proceedings of the Linnean Society of New South Wales'; break;
			case 'PKNAW': case 'Proceedings of the Koninklijke Nederlandse Academie van Wetenschappen': $o = 'Proceedings of the Koninklijke Nederlandse Akademie van Wetenschappen'; break;
			case 'PN': case 'Palaeontologica Nova, Seminario de Paleontología de Zaragoza': $o = 'Palaeontologica Nova'; break;
			case 'PNAS': case 'Proceedings of the National Academy of Sciences of the United States of America': $o = 'Proceedings of the National Academy of Sciences'; break;
			case 'POP': case 'Papers on Paleontology, The Museum of Palaeontology, University of Michigan': $o = 'Papers on Paleontology'; break;
			case 'PPP': case 'Palaeogeography,Palaeoclimatology,Palaeoecology': $o = 'Palaeogeography, Palaeoclimatology, Palaeoecology'; break;
			case 'PZ': case 'Paläontologisches Zeitschrift': $o = 'Paläontologische Zeitschrift'; break;
			case 'QSR': case 'Quaternary Science Review': $o = 'Quaternary Science Reviews'; break;
			case 'RAPBBA': $o = 'RAP Bulletin of Biological Assessment'; break;
			case 'RBG': case 'Rev. Brasil. Genet.': $o = 'Revista Brasileira de Genetica'; break;
			case 'RJT': $o = 'Russian Journal of Theriology'; break;
			case 'RSGE': $o = 'Revista de la Sociedad Geológica de España'; break;
			case 'RWAM': $o = 'Records of the Western Australian Museum'; break;
			case 'RZA': case 'Revue de Zoologie africaine': $o = 'Revue Zoologique africaine'; break;
			case 'Science, New Series': $o = 'Science'; break;
			case 'SBN': $o = 'Stuttgarter Beiträge zur Naturkunde'; break;
			case 'SCC': $o = 'Small Carnivore Conservation'; break;
			case 'SCZ': $o = 'Smithsonian Contributions to Zoology'; break;
			case 'SG': $o = 'Scripta Geologica'; break;
			case 'TMGB': $o = 'Treballs del Museu de Geologia de Barcelona'; break;
			case 'VPA': $o = 'Vertebrata PalAsiatica'; break;
			case 'VZ': case 'Vestnik Zoologii': $o = 'Vestnik zoologii'; break;
			case 'WNAN': $o = 'Western North American Naturalist'; break;
			case 'ZA': case 'Zoologischer Anzeiger - A Journal of Comparative Zoology': case 'Zoologischer Anzeiger, Jena': $o = 'Zoologischer Anzeiger'; break;
			case 'ZM': case 'Zoologische Mededelingen Leiden': case 'Zoölogische Mededelingen': $o = 'Zoologische Mededelingen'; break;
			case 'ZME': $o = 'Zoology in the Middle East'; break;
			default: $o = $in; break;
		}
		if($in)
			return $o;
		else
			if($o) $this->journal = $o;
	}
	/* KINDS OF FILES AND CONSEQUENCES */
	public function isfullissue() {
		return $this->fullissue ? true : false;
	}
	public function iserratum() {
		return (strpos($this->name, 'erratum') !== false);
	}
	public function isnopagenumberjournal() {
	// these journals don't have page numbers, only "volume" and "issue". They should get rid of their failure to implement standard practices, but in the meantime we have to handle them.
		return in_array($this->journal,
			array('Palaeontologia Electronica',
				'BMC Biology',
				'BMC Evolutionary Biology',
				'PLoS ONE',
				'PLoS Biology',
				'Genome Biology',
				'Canid News',
				'Frontiers in Zoology',
				'Journal of Biomedicine and Biotechnology',
		));
	}
	public function isfile() {
	// returns whether this 'file' is a file
		return ($this->folder !== 'NOFILE');
	}
	public function isamnh() {
		return in_array(
			$this->journal,
			array(
				'American Museum Novitates',
				'Bulletin of the American Museum of Natural History',
				'Anthropological Papers of the American Museum of the Natural History',
				'Memoirs of the American Museum of Natural History',
				)
			);
	}
	private function hasid() {
	// whether this file has any kind of online ID
		return ($this->url or $this->doi or $this->jstor or $this->hdl or $this->pmid or $this->pmc);
	}
	/* MANUAL EDITING */
	public function set(array $paras) {
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'checklist' => array(
				'cannotmove' => 'Whether to disallow moving a page',
			),
			'checkfunc' => function($in) {
				return true;
			},
			'default' => array('cannotmove' => false),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		foreach($paras as $field => $content) {
			if(self::hasproperty($field)) {
				if($this->$field === $content) continue;
				switch($field) {
					case 'parent':
						if(strlen($content) > 0 && !$this->p->has($content)) {
							echo 'Adding the enclosing file...' . PHP_EOL;
							$this->p->addNofile(array('handle' => $content));
						}
						$this->parent = $content;
						break;
					case 'name':
						if(!$paras['cannotmove']) {
							$this->move($content);
						}
						break;
					default:
						$this->$field = $content;
						break;
				}
				$this->needSave();
			}
		}
	}
	/* CITING */
	protected function _getAuthors() {
		$authors = explode('; ', $this->authors);
		return array_map(function($author) {
			return explode(', ', $author);
		}, $authors);
	}
	/* ADDING DATA */
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
				echo 'Suggested placement. Folder: ' . $sugg[0];
				if($sugg[1] !== '') {
					echo '; subfolder: ' . $sugg[1];
					if($sugg[2] !== '') {
						echo '; sub-subfolder: ' . $sugg[2];
					}
				}
				$cmd = $this->menu(array(
					'head' => PHP_EOL,
					'options' => array(
						'y' => 'this suggestion is correct',
						'n' => 'this suggestion is not correct',
						's' => 'stop suggestions',
						'q' => 'quit this file',
					),
				));
				switch($cmd) {
					case 'y':
						$this->folder = $sugg[0];
						$this->sfolder = $sugg[1];
						$this->ssfolder = $sugg[2];
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
		/* folder */
		echo 'Suggestions: ';
		$suggs = $sugg_lister($foldertree);
		$this->folder = $folder = $this->menu($menuOptions);
		/* subfolder */
		if($folder !== '' && count($foldertree[$folder]) !== 0) {
			echo 'Suggestions: ';
			$foldertree = $this->p->foldertree[$this->folder];
			$suggs = $sugg_lister($this->p->foldertree[$this->folder]);
			// update menu options
			$menuOptions['head'] = 'Subfolder: ';
			$this->sfolder = $sfolder = $this->menu($menuOptions);
			/* sub-subfolder */
			if($sfolder !== '' && count($foldertree[$sfolder]) !== 0) {
				echo 'Suggestions: ';
				$foldertree = $foldertree[$sfolder];
				$suggs = $sugg_lister($foldertree);
				$menuOptions['head'] = 'Sub-subfolder: ';
				$this->ssfolder = $this->menu($menuOptions);
			}
		}
		return true;
	}
	protected function setCurrentDate() {
		// add time added
		$time = getdate();
		$this->addmonth = $time["mon"];
		$this->addday = $time["mday"];
		$this->addyear = $time["year"];
		return true;
	}
	protected function manuallyFillProperty($field) {
		$result = $this->fillScalarProperty($field);
		// this space may be used to set some properties on the basis of values of others
		return $result;
	}
	/* SMALL UTILITIES */
	public function needsdata() {
	// whether this citation has incomplete information
		return !($this->hasid() and $this->year and $this->volume and $this->start_page);
	}
	public function setpath(array $paras = array()) {
	// set the path for a file on the basis of a fromfile. This can now be used
	// in lscheck; as a similar process becomes useful elsewhere, this function
	// may be expanded.
		if($this->process_paras($paras, array(
			'name' => __FUNCTION__,
			'synonyms' => array('v' => 'verbose'),
			'checklist' => array(
				'fromfile' => 'File to copy path from',
				'verbose' => 'Whether to provide verbose output',
			),
			'default' => array('verbose' => true),
			'errorifempty' => array('fromfile'),
			'checkparas' => array(
				'fromfile' => function($in) {
					return $in instanceof self;
				},
			),
		)) === PROCESS_PARAS_ERROR_FOUND) return false;
		$once = false;
		foreach(array('folder', 'sfolder', 'ssfolder') as $v) {
			if($this->$v !== $paras['fromfile']->$v) {
				if($paras['verbose']) {
					if(!$once) {
						echo 'Updating folders for file ' . $this->name . PHP_EOL;
						$once = true;
					}
					echo 'Stored ' . $v . ': ';
					Sanitizer::printVar($this->$v);
					echo PHP_EOL;
					echo 'New ' . $v . ': '; 
					Sanitizer::printVar($paras['fromfile']->$v); 
					echo PHP_EOL;
				}
				$this->$v = $paras['fromfile']->$v;
			}
		}
		return true;
	}
	/*
	 * Helper functions to do with enclosing
	 */
	// TODO: this is used in expanddoi
	private function fillEnclosingFromTitle($title) {
		echo 'Enclosing title: ' . $title . PHP_EOL;
		$candidates = $this->p->bfind(array(
			'title' => $title,
		));
		foreach($candidates as $candidate) {
			$candidate->inform();
			if($this->ynmenu("Is this the correct book?")) {
				$this->parent = $candidate->name;
				return true;
			}
		}
		// now, ask the user
		$cmd = $this->menu(array(
			'head' => 'If possible, enter the handle of the enclosing entry',
			'options' => array('n' => 'Make a new entry'),
			'validfunction' => function($in) {
				if($in === 'n') {
					return true;
				} else {
					return $this->p->has($in);
				}
			}
		));
		if($cmd !== 'n') {
			$this->parent = $cmd;
			return true;
		}
		// now, make a new one
		$newName = $this->menu(array(
			'head' => 'We will create a new entry for the enclosing. Enter a handle:',
			'validfunction' => function($in) {
				return !$this->p->has($in);
			}
		));
		$entry = new self(NULL, 'e', $this->p);
		$entry->setCurrentDate();
		$entry->folder = 'NOFILE';
		$entry->name = $newName;
		$entry->title = $title;
		$this->p->addEntry($entry, array('isnew' => true));
		$this->parent = $newName;
		return true;
	}
	
	/*
	 * Fields
	 */
	protected static function fillFields() {
		return array(
			'name' => new CsvProperty(array(
				'name' => 'name',
				'type' => Property::STRING)),
			'folder' => new CsvProperty(array(
				'name' => 'folder',
				'type' => Property::STRING)),
			'sfolder' => new CsvProperty(array(
				'name' => 'folder',
				'type' => Property::STRING)),
			'ssfolder' => new CsvProperty(array(
				'name' => 'folder',
				'type' => Property::STRING)),
			'addmonth' => new CsvProperty(array(
				'name' => 'added',
				'type' => Property::INT)),
			'addday' => new CsvProperty(array(
				'name' => 'added',
				'type' => Property::INT)),
			'addyear' => new CsvProperty(array(
				'name' => 'added',
				'type' => Property::INT)),
			'type' => new CsvProperty(array(
				'name' => 'type',
				'validator' => function($in) {
					// this is an enum, so check whether it has an allowed value
					return true;
				},
				'type' => Property::INT)),
			'authors' => new CsvProperty(array(
				'name' => 'authors',
				'type' => Property::STRING)),
			'year' => new CsvProperty(array(
				'name' => 'year',
				'validator' => function($in) {
					return preg_match('/^(\d+|undated|\d+–\d+)$/', $in);
				},
				'type' => Property::STRING)),
			'title' => new CsvProperty(array(
				'name' => 'title',
				'type' => Property::STRING)),
			'journal' => new CsvProperty(array(
				'name' => 'journal',
				'type' => Property::STRING)),
			'series' => new CsvProperty(array(
				'name' => 'series',
				'type' => Property::STRING)),
			'volume' => new CsvProperty(array(
				'name' => 'volume',
				'type' => Property::STRING)),
			'issue' => new CsvProperty(array(
				'name' => 'issue',
				'type' => Property::STRING)),
			'start_page' => new CsvProperty(array(
				'name' => 'start_page',
				'type' => Property::STRING)),
			'end_page' => new CsvProperty(array(
				'name' => 'end_page',
				'type' => Property::STRING)),
			'pages' => new CsvProperty(array(
				'name' => 'pages',
				'type' => Property::STRING)),
			'url' => new CsvProperty(array(
				'name' => 'url',
				'type' => Property::STRING)),
			'doi' => new CsvProperty(array(
				'name' => 'doi',
				'type' => Property::STRING)),
			'parent' => new CsvProperty(array(
				'name' => 'parent',
				'type' => Property::STRING)),
			'publisher' => new CsvProperty(array(
				'name' => 'publisher',
				'type' => Property::STRING)),
			'misc_data' => new CsvProperty(array(
				'name' => 'misc_data',
				'type' => Property::STRING)),
		);
	}
}

CsvArticle::init();
