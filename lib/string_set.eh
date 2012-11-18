#!/usr/bin/ehi
class StringSet
	private hash

	public initialize = () => (this.hash = {})

	public empty = this.new

	public add = func: elt
		this.hash->elt = true
	end

	public has = elt => this.hash.has elt

	public each = func: f
		for (key, value) in this.hash
			f key
		end
	end
	
	public remove = elt => (this.hash.delete elt)

	public union = func: other
		other.each this.add
	end

	public intersection = func: other
		out = empty()
		this.each func: elt
			if other.has elt
				out.add elt
			end
		end
	end
	
	public toString = func:
		private it = this.hash.getIterator()
		out = '{'
		while(it.hasNext())
			elt, () = it.next()
			out += elt
			if(it.hasNext())
				out += ', '
			end
		end
		out + '}'
	end
end
