#!/usr/bin/php
<?php
/*
 * printpdf.php - Prints the first page of a PDF using Article methods
 * Used to help develop Article's PDF-reading capabilities
 */
if($argc !== 2) die("No filename specified" . PHP_EOL);
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Common/CsvContainerList.php");
require_once(BPATH . "/Catalog/Article.php");
require_once(BPATH . "/Catalog/settings.php");
$ff = new Article();
$ff->name = $argv[1];
$ff->echopdfcontent();
