<?php
require_once(__DIR__ . '/../Common/common.php');

class LocationList extends SqlContainerList {
	static $childClass = 'Location';
	public function table() {
		return 'location';
	}
}
