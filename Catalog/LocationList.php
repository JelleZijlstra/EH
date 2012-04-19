<?php
require_once(__DIR__ . '/../Common/common.php');

class LocationList extends SqlContainerList {
	protected function table() {
		return 'location';
	}
}
