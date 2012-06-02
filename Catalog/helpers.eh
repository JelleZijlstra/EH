#!/usr/bin/ehi
# Functions that are of use in ArticleList

# Tiny finder
f := func: in {
	bfind --title='/' . $in . '/i'
}

# Fix Annals and Magazine of Natural History series and journal name
fixannals := func: dryRun {
	for $(bfind -q --journal='/Journal of Natural History Series \d+/') as file {
		journal := $(getField $file --field=journal)
		series := $(call preg_replace ['/^.*(\d+)$/', '$1', $journal])
		if $dryRun {
			echo $(getField $file --field=name)
			echo $series
		} else {
			setprops $file --journal='Annals and Magazine of Natural History' --series=$series
		}
	}
}
