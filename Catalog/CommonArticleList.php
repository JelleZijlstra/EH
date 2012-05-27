<?php
require_once(BPATH . '/Catalog/load.php');

trait CommonArticleList {
	abstract public function addRedirect(array $paras);
}
