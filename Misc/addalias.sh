#!/bin/bash
# addalias.sh
# Add a persistent alias in bash
if [ -z $2 ]; then
	echo 'Invalid input'
	exit
fi
alias $1='$2'
echo "alias $1='$2'" >> ~/.bash_profile
exit
