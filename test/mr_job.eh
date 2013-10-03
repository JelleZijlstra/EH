#!/usr/bin/ehi

include '../lib/map_reduce.eh'
include '../lib/hash.eh'

class WordCounter
	this.inherit MapReduceJob

	mapper(key, value) = for word in value
		this.emit(word, 1)
	end

	reducer(key, values) = this.output(key, values.length())
end

arr = [['hello', 'world'], ['goodbye', 'world']]
echo (WordCounter().run arr)
