<?php
/*
 * This is the entry point for loading catalog.csv
 * require_once('load.php'); will load the catalog
 */
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Container/CsvContainerList.php');
require_once(BPATH . '/Parse/parser.php');
require_once(BPATH . '/List/settings.php');
require_once(BPATH . '/List/Country.php');
require_once(BPATH . '/List/Taxon.php');
require_once(BPATH . '/List/TaxonList.php');
$taxonlist = array(); // needed to make ListEntry::$p kind of work.
$taxonlist = new TaxonList();
?>
