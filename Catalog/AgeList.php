<?php
require_once(__DIR__ . '/../Common/common.php');

class AgeList extends SqlContainerList {
	static $childClass = 'Age';
	protected function table() {
		return 'age';
	}
}
