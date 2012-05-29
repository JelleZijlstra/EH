<?php
/*
 * Article.php
 *
 * A class to represent a single library file with its bibliographic data.
 * An Article should be a member of the $p array of a ArticleList object.
 */

class Article extends SqlListEntry implements ArticleInterface {
	use CommonArticle;
	
	// properties specific to Article
	protected /* int */ $id;
	protected /* Folder */ $folder;
	protected /* string */ $added;
	protected /* string array */ $identifiers;
	
	protected function publisher() {
		return (string) $this->publisher;
	}
	protected function journal() {
		return (string) $this->journal;
	}
	protected function location() {
		if($this->publisher === NULL) {
			return false;
		} else {
			return (string) $this->publisher->location;
		}
	}
	protected function doi() {
		return $this->getIdentifier('doi');
	}
	protected function hdl() {
		return $this->getIdentifier('hdl');
	}
	protected function jstor() {
		return $this->getIdentifier('jstor');
	}
	protected function _getIdentifier($name) {
		if(isset($this->identifiers[$name])) {
			return $this->identifiers[$name];
		} else {
			return false;
		}
	}
	/*
	 * Constructors
	 */
	public static function makeNewNoFile(/* string */ $handle, ContainerList $parent) {
		// TODO
	}
	public static function makeNewRedirect(/* string */ $handle, $target, ContainerList $parent) {
		// TODO
	}
	/*
	 * Basic operations
	 */
	protected function _path(array $paras) {
		$this->fillProperties();
		return $this->folder->path();
	}
	public function isfile() {
		$this->fillProperties();
		return $this->folder !== NULL;
	}
	protected function _getAuthors() {
		$this->fillProperties();
		return array_map(function($author) {
			return $author->toArray();
		}, $this->authors);
	}
	protected function manuallyFillProperty($field) {
		$result = $this->fillScalarProperty($field);
		// this space may be used to set some properties on the basis of values of others
		return $result;
	}
	protected function setCurrentDate() {
		$date = new DateTime();
		$this->added = $date->format('Y-m-d H:i:s');
	}
	protected function determinePath() {
	
	}
	protected function setPathFromArray($in) {
		assert(isset($in[0]));
		$f = array_shift($in);
		$parentFolder = Folder::withName($f);
		$this->folder = $parentFolder->makeChildPath($in);
	}
	/*
	 * Identifiers
	 */
	const URL = 1;
	const DOI = 2;
	const JSTOR = 3;
	const HDL = 4;
	const PMID = 5;
	const PMC = 6;
	const EUROBATS = 7;
	const EDITION = 8;
	public static function identifierToString($in) {
		switch($in) {
			case self::URL: return 'url';
			case self::DOI: return 'doi';
			case self::JSTOR: return 'jstor';
			case self::HDL: return 'hdl';
			case self::PMID: return 'pmid';
			case self::PMC: return 'pmc';
			case self::EUROBATS: return 'eurobats';
			case self::EDITION: return 'edition';			
			default: throw new EHInvalidArgumentException($in);
		}
	}
	public static function stringToIdentifier($in) {
		switch($in) {
			case 'url': return self::URL;
			case 'doi': return self::DOI;
			case 'jstor': return self::JSTOR;
			case 'hdl': return self::HDL;
			case 'pmid': return self::PMID;
			case 'pmc': return self::PMC;
			case 'eurobats': return self::EUROBATS;
			case 'edition': return self::EDITION;
			default: throw new EHInvalidArgumentException($in);
		}
	}
	
	/*
	 * Fields
	 */
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => Property::ID)),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => Property::STRING)),
			'folder' => new SqlProperty(array(
				'name' => 'folder',
				'type' => Property::REFERENCE,
				'referredClass' => 'Folder')),
			'added' => new SqlProperty(array(
				'name' => 'added',
				'type' => Property::TIMESTAMP)),
			'type' => new SqlProperty(array(
				'name' => 'type',
				'validator' => function($in) {
					// this is an enum, so check whether it has an allowed value
					// TODO
					return true;
				},
				'type' => Property::INT)),
			'year' => new SqlProperty(array(
				'name' => 'year',
				'validator' => function($in) {
					return preg_match('/^(\d+|undated|\d+–\d+)$/', $in);
				},
				'type' => Property::STRING)),
			'title' => new SqlProperty(array(
				'name' => 'title',
				'type' => Property::STRING)),
			'journal' => new SqlProperty(array(
				'name' => 'journal',
				'type' => Property::REFERENCE,
				'referredClass' => 'journal')),
			'series' => new SqlProperty(array(
				'name' => 'series',
				'type' => Property::STRING)),
			'volume' => new SqlProperty(array(
				'name' => 'volume',
				'type' => Property::STRING)),
			'issue' => new SqlProperty(array(
				'name' => 'issue',
				'type' => Property::STRING)),
			'start_page' => new SqlProperty(array(
				'name' => 'start_page',
				'type' => Property::STRING)),
			'end_page' => new SqlProperty(array(
				'name' => 'end_page',
				'type' => Property::STRING)),
			'pages' => new SqlProperty(array(
				'name' => 'pages',
				'type' => Property::STRING)),
			'parent' => new SqlProperty(array(
				'name' => 'parent',
				'type' => Property::REFERENCE,
				'referredClass' => 'Article')),
			'publisher' => new SqlProperty(array(
				'name' => 'publisher',
				'type' => Property::REFERENCE,
				'referredClass' => 'Publisher')),
			'misc_data' => new SqlProperty(array(
				'name' => 'misc_data',
				'type' => Property::STRING)),
			'authors' => new SqlProperty(array(
				'name' => 'authors',
				'type' => Property::CUSTOM,
				'automatedFiller' => function($id) {
					$authors = Database::singleton()->select(array(
						'fields' => array('author_id'),
						'from' => 'article_author',
						'where' => array(
							'article_id' => $id,
						),
						'order_by' => 'position',
					));
					$out = array();
					foreach($authors as $author) {
						$out[] = Author::withId($author['article_id']);
					}
					return $out;
				})),
			'identifiers' => new SqlProperty(array(
				'name' => 'identifiers',
				'type' => Property::CUSTOM,
				'automatedFiller' => function($id) {
					$identifiers = Database::singleton()->select(array(
						'fields' => array('data', 'identifier'),
						'from' => 'article_identifier',
						'where' => array(
							'article_id' => $id,
						),
					));
					$out = array();
					foreach($identifiers as $identifier) {
						$name = Article::identifierToString(
							$identifier['identifier']);
						$out[$name] = $identifier['data'];
					}
					return $out;
				})),
		);
	}
}
