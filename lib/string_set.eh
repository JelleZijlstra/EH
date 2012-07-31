#!/usr/bin/ehi
class StringSet
	initialize = func: -> {}

	empty = func: -> new()

	add = func: elt
		self->elt = true
	end

	has = func: elt -> (self.has elt)

	each = func: f
		for self as key => value
			f key
		end
	end

	union = func: other
		other.each func: elt -> (add elt)
	end

	intersection = func: other
		out = empty()
		each func: elt
			if (other.has elt)
				out.add elt
			end
		end
	end
end
