# EH MapReduce library

class MapReduceJob
	private intermediates
	private outputArray

	const public run = func: input
		if this.mapper == null
			throw "Mapper is null"
		end
		if this.reducer == null
			throw "Reducer is null"
		end
		this.intermediates = []
		for (key, value) in input
			this.mapper key, value
		end
		this.outputArray = []
		for (key, values) in this.intermediates
			this.reducer key, values
		end
		this.outputArray
	end
	
	emit = func: key, value
		if !(this.intermediates.has key)
			this.intermediates->key = []
		end
		this.intermediates->key.append value
	end
	
	output = func: key, value
		this.outputArray->key = value
	end
	
	combiner = null
	
	mapper = null
	
	reducer = null
end
