<?php
require_once(__DIR__ . '/../Common/common.php');

class FolderList extends SqlContainerList {
	protected function table() {
		return 'folder';
	}
}
