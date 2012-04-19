<?php
require_once(__DIR__ . '/../Common/common.php');

class JournalList extends SqlContainerList {
	protected function table() {
		return 'journals';
	}
}
