#!/usr/bin/php
<?php
/*
 * printpdf.php - Prints the first page of a PDF using FullFile methods
 * Used to help develop FullFile's PDF-reading capabilities
 */
if($argc !== 2) die("No filename specified" . PHP_EOL);
require_once(__DIR__ . "/../Common/common.php");
require_once(BPATH . "/Common/List.php");
require_once(BPATH . "/Catalog/FullFile.php");
require_once(BPATH . "/Catalog/settings.php");
$ff = new FullFile();
$ff->name = $argv[1];
$ff->echopdfcontent();
