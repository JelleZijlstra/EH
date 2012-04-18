<?php
/*
 * This is the entry point for loading catalog.csv
 * require_once('load.php'); will load the catalog
 *
 * Things to do to improve the code:
 * - Get rid as much as possible of the use of ArticleList properties in the Article class, which are "pseudoglobals"
 * - Improve pdfcontent_findtitle to include some of the logic of the trygoogle() etc methods
 */
require_once(__DIR__ . '/../Common/common.php');
require_once(BPATH . '/Container/CsvContainerList.php');
require_once(BPATH . '/Catalog/settings.php');
require_once(BPATH . '/Catalog/Article.php');
require_once(BPATH . '/Catalog/ArticleList.php');
require_once(BPATH . '/Parse/parser.php');
// Without this line, switchcli doesn't work. Don't ask me why.
//$csvlist = NULL;
$csvlist = ArticleList::singleton();
