<?php
require_once(__DIR__ . '/../Common/common.php');

class OccurrenceList extends SqlContainerList {
	protected function table() {
		return 'occurrence';
	}
}
