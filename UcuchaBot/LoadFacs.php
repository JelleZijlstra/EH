<?php
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/UcuchaBot/Facs.php');
$FacsList = new FacsList();
$FacsList->cli();
