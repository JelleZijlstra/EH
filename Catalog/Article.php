<?php
/*
 * Article.php
 *
 * A class to represent a single library file with its bibliographic data.
 * An Article should be a member of the $p array of a ArticleList object.
 */

class Article extends SqlListEntry implements ArticleInterface {
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
	protected function journal() {
		return $this->journal->name();
	}
	/*
	 * Citing
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
					return true;
				},
				'type' => Property::INT)),
			'authors' => new SqlProperty(array(
				'name' => 'authors',
				'type' => Property::CUSTOM,
				'automatedFiller' => function($id) {
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
			'url' => new SqlProperty(array(
				'name' => 'url',
				'type' => Property::STRING)),
			'doi' => new SqlProperty(array(
				'name' => 'doi',
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
			'children' => new SqlProperty(array(
				'name' => 'children',
				'type' => Property::CHILDREN)),
		);
	}
}
