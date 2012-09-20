#!/usr/bin/ehi
class StringSet
	private hash

	initialize = func: -> (this.hash = {})

	empty = func: -> (this.new())

	add = func: elt
		this.hash->elt = true
	end

	has = func: elt -> (this.hash.has elt)

	each = func: f
		for (key, value) in this.hash
			f key
		end
	end

	union = func: other
		other.each this.add
	end

	intersection = func: other
		out = empty()
		this.each func: elt
			if (other.has elt)
				out.add elt
			end
		end
	end
end
