<?php
require_once(__DIR__ . '/../Common/common.php');

class AuthorList extends SqlContainerList {
	protected function table() {
		return 'author';
	}
}
