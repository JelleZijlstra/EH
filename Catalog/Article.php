<?php
/*
 * Article.php
 *
 * A class to represent a single library file with its bibliographic data.
 * An Article should be a member of the $p array of a ArticleList object.
 */

class Article extends SqlListEntry {
	use CommonArticle;
	
	protected /* int */ $id;
	protected /* string */ $name;
	protected /* Folder */ $folder;
	protected /* string */ $added;
	protected /* int */ $type;
	protected /* Author array */ $authors;
	protected /* string|int */ $year;
	protected /* string */ $title;
	protected /* Journal */ $journal;
	protected /* string */ $series;
	protected /* string */ $volume;
	protected /* string */ $issue;
	protected /* string */ $start_page;
	protected /* string */ $end_page;
	// TODO: more properties
	
	protected function publisher() {
		return $this->publisher->name();
	}
	
	/*
	 * Types.
	 */
	const JOURNAL = 9; // no zero because that value may signify an error
	const CHAPTER = 1;
	const BOOK = 2;
	const THESIS = 3; // kind of degree in "series", university in "publisher"
	const WEB = 5;
	const MISCELLANEOUS = 6;
	const REDIRECT = 7; // ID of target in "parent"
	const SUPPLEMENT = 8; // ID of target in "parent", kind of supplement in "title"

	/*
	 * Kinds of files
	 */
	public function issupplement() {
		return $this->type === self::SUPPLEMENT;
	}

	/*
	 * SqlListEntry stuff.
	 */
	protected static function fillFields() {
		return array(
			'id' => new SqlProperty(array(
				'name' => 'id',
				'type' => SqlProperty::ID)),
			'name' => new SqlProperty(array(
				'name' => 'name',
				'type' => SqlProperty::STRING)),
			'folder' => new SqlProperty(array(
				'name' => 'folder',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Folder')),
			'added' => new SqlProperty(array(
				'name' => 'added',
				'type' => SqlProperty::TIMESTAMP)),
			'type' => new SqlProperty(array(
				'name' => 'type',
				'validator' => function($in) {
					// this is an enum, so check whether it has an allowed value
					return true;
				},
				'type' => SqlProperty::INT)),
			'authors' => new SqlProperty(array(
				'name' => 'authors',
				'type' => SqlProperty::CUSTOM,
				'creator' => function($id) {
					$authors = Database::singleton()->select(array(
						'fields' => array('author_id'),
						'from' => 'article_author',
						'where' => array(
							'article_id' => Database::escapeValue($id)
						),
						'order_by' => 'position',
					));
					$out = array();
					foreach($authors as $author) {
						$out[] = Author::withId($author['article_id']);
					}
					return $out;
				})),
			'year' => new SqlProperty(array(
				'name' => 'year',
				'validator' => function($in) {
					return preg_match('/^(\d+|undated|\d+–\d+)$/', $in);
				},
				'type' => SqlProperty::STRING)),
			'title' => new SqlProperty(array(
				'name' => 'title',
				'type' => SqlProperty::STRING)),
			'journal' => new SqlProperty(array(
				'name' => 'journal',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'journal')),
			'series' => new SqlProperty(array(
				'name' => 'series',
				'type' => SqlProperty::STRING)),
			'volume' => new SqlProperty(array(
				'name' => 'volume',
				'type' => SqlProperty::STRING)),
			'issue' => new SqlProperty(array(
				'name' => 'issue',
				'type' => SqlProperty::STRING)),
			'start_page' => new SqlProperty(array(
				'name' => 'start_page',
				'type' => SqlProperty::STRING)),
			'end_page' => new SqlProperty(array(
				'name' => 'end_page',
				'type' => SqlProperty::STRING)),
			'pages' => new SqlProperty(array(
				'name' => 'pages',
				'type' => SqlProperty::STRING)),
			'url' => new SqlProperty(array(
				'name' => 'url',
				'type' => SqlProperty::STRING)),
			'doi' => new SqlProperty(array(
				'name' => 'doi',
				'type' => SqlProperty::STRING)),
			'parent' => new SqlProperty(array(
				'name' => 'parent',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Article')),
			'publisher' => new SqlProperty(array(
				'name' => 'publisher',
				'type' => SqlProperty::REFERENCE,
				'referredClass' => 'Publisher')),
			'part_identifier' => new SqlProperty(array(
				'name' => 'part_identifier',
				'type' => SqlProperty::BOOL)),
			'misc_data' => new SqlProperty(array(
				'name' => 'misc_data',
				'type' => SqlProperty::STRING)),
			'children' => new SqlProperty(array(
				'name' => 'children',
				'type' => SqlProperty::CHILDREN)),
		);
	}

}
