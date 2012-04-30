<?php
require_once(__DIR__ . '/../Common/common.php');

class PublisherList extends SqlContainerList {
	static $childClass = 'Publisher';
	public function table() {
		return 'publisher';
	}
}
