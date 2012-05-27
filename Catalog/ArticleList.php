<?php
require_once(__DIR__ . '/../Common/common.php');

class ArticleList extends SqlContainerList {
	use CommonArticleList;
	
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
