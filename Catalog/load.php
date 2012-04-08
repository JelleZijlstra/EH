<?php
/*
 * This is the entry point for loading catalog.csv
 * require_once('load.php'); will load the catalog
 *
 * Things to do to improve the code:
 * - Get rid as much as possible of the use of CsvList properties in the FullFile class, which are "pseudoglobals"
 * - Improve pdfcontent_findtitle to include some of the logic of the trygoogle() etc methods
 */
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Container/CsvContainerList.php');
require_once(BPATH . '/Catalog/settings.php');
require_once(BPATH . '/Catalog/FullFile.php');
require_once(BPATH . '/Catalog/CsvList.php');
require_once(BPATH . '/Parse/parser.php');
$csvlist = array(); // this line is needed to remove some bugs involving __set and FullFile::p. Without it, fatal errors may occur randomly.
$csvlist = new CsvList();
?>
