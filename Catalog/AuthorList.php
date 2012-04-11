<?php
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Container/SqlContainerList.php');
require_once(BPATH . '/Catalog/Author.php');

class AuthorList extends SqlContainerList {
	protected function table() {
		return 'author';
	}
}
