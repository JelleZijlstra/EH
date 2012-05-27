<?php
require_once(__DIR__ . '/../Common/common.php');

class ArticleList extends SqlContainerList {
	
	/*
	 * SqlContainerList things.
	 */
	public function table() {
		return 'article';
	}
}
