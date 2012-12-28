#!/usr/bin/ehi

include '../lib/map_reduce.eh'
include '../lib/array.eh'

class WordCounter
	this.inherit MapReduceJob

	mapper = func: key, value
		for _, word in value
			this.emit(word, 1)
		end
	end

	reducer = func: key, values
		this.output(key, values.length())
	end
end

arr = [['hello', 'world'], ['goodbye', 'world']]
echo (WordCounter.run arr)
