<?php
require_once(__DIR__ . '/../Common/common.php');

class FolderList extends SqlContainerList {
	static $childClass = 'folder';
	protected function table() {
		return 'folder';
	}
}
