<?php
require_once(__DIR__ . '/../Common/common.php');

class OccurrenceList extends SqlContainerList {
	static $childClass = 'Occurrence';
	protected function table() {
		return 'occurrence';
	}
}
