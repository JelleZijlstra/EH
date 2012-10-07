#!/usr/bin/ehi
include '../lib/library.eh'

echo "foo".reverse()
echo (1::2::Nil).reverse()

echo "foo".map x => x + x
