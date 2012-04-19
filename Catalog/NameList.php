<?php
require_once(__DIR__ . '/../Common/common.php');

class NameList extends SqlContainerList {
	protected function table() {
		return 'name';
	}
}
