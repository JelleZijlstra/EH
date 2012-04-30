<?php
require_once(__DIR__ . '/../Common/common.php');

class AgeList extends SqlContainerList {
	static $childClass = 'Age';
	public function table() {
		return 'age';
	}
}
