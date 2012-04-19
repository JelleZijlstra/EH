<?php
require_once(__DIR__ . '/../Common/common.php');

class PublisherList extends SqlContainerList {
	protected function table() {
		return 'publisher';
	}
}
