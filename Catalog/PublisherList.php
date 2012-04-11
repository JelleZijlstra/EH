<?php
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Container/SqlContainerList.php');
require_once(BPATH . '/Catalog/Publisher.php');

class PublisherList extends SqlContainerList {
	protected function table() {
		return 'publisher';
	}
}
