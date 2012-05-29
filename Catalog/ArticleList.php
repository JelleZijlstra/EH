<?php
require_once(__DIR__ . '/../Common/common.php');

class ArticleList extends SqlContainerList {
	use CommonArticleList;
	
	static $childClass = 'Article';

	protected function __construct(array $commands = array()) {
		parent::__construct(self::$ArticleList_commands);
	}
	
	public function addRedirect(array $paras) {
		//TODO
	}
	
	/*
	 * SqlContainerList things.
	 */
	public function table() {
		return 'article';
	}
}
