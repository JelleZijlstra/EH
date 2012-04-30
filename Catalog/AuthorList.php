<?php
require_once(__DIR__ . '/../Common/common.php');

class AuthorList extends SqlContainerList {
	static $childClass = 'author';
	public function table() {
		return 'author';
	}
}
