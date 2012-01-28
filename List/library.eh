#!/usr/bin/ehi
# A library of useful functions for List

# Find taxa without a citation from after a given year
func nocite: year
	bfind --citation='Unknown' --year=">" + @string $year
end
