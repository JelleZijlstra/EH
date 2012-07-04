<?php
/*
 * load.php - Loads a GeneralList instance (which automatically starts its own CLI)
 */
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/GeneralList/GeneralList.php');
$generallist = new GeneralList();
