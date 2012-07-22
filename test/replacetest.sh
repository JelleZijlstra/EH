#!/bin/bash
for var in "$@"; do
	ehi "$var.eh" > "$var.expected" 2>&1
done
