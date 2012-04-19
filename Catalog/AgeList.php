<?php
require_once(__DIR__ . '/../Common/common.php');

class AgeList extends SqlContainerList {
	protected function table() {
		return 'age';
	}
}
