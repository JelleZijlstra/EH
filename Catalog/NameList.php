<?php
require_once(__DIR__ . '/../Common/common.php');

class NameList extends SqlContainerList {
	static $childClass = 'Name';
	public function table() {
		return 'name';
	}
}
