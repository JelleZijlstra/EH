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
	protected function cite_getclass() {
		switch($this->type) {
			case self::JOURNAL:
				return 'journal';
			case self::CHAPTER:
				return 'chapter';
			case self::BOOK:
				return 'book';
			case self::THESIS:
				return 'thesis';
			case self::WEB:
				return 'web';
			case self::MISCELLANEOUS:
				return 'unknown';
			case self::REDIRECT:
			case self::SUPPLEMENT:
				return 'n/a';
			default:
				throw new EHException('Invalid type: ' . $this->type);
		}
	}
}
