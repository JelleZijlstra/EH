#!/bin/bash
# latexpdf.sh - Convert directly from Latex to PDF
texfile=$@
if [ ${texfile:(-4):4} != ".tex" ]
then
	echo 'Invalid input file'
	exit
fi
# logfile=${infile/.tex/.shl}
plainfile=${texfile/.tex/}
latex $texfile
auxfile=${texfile/.tex/.aux}
bibtex $plainfile
latex $texfile
latex $texfile
dvifile=${texfile/.tex/.dvi}
dvipdfm $dvifile
pdffile=${texfile/.tex/.pdf}
open $pdffile
