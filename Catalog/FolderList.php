<?php
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Container/SqlContainerList.php');
require_once(BPATH . '/Catalog/Folder.php');

class FolderList extends SqlContainerList {
	protected function table() {
		return 'folder';
	}
}
