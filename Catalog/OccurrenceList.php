<?php
require_once(__DIR__ . '/../Common/common.php');

class OccurrenceList extends SqlContainerList {
	static $childClass = 'Occurrence';
	public function table() {
		return 'occurrence';
	}
}
