<?php
require_once(__DIR__ . '/../Common/common.php');

class CityList extends SqlContainerList {
	protected function table() {
		return 'city';
	}
}
