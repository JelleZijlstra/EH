<?php
require_once(__DIR__ . '/../Common/common.php');

class JournalList extends SqlContainerList {
	static $childClass = 'Journal';
	public function table() {
		return 'journal';
	}
}
