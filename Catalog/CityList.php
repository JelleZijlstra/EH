<?php
require_once(__DIR__ . '/../Common/common.php');

class CityList extends SqlContainerList {
	static $childClass = 'City';
	public function table() {
		return 'city';
	}
}
