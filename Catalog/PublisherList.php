<?php
require_once(__DIR__ . '/../Common/common.php');

class PublisherList extends SqlContainerList {
	static $childClass = 'Publisher';
	protected function table() {
		return 'publisher';
	}
}
