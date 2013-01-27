# JSON utilities

JSON.parseFile = name => JSON.parse(File.readFile name)

Integer.toJSON = Integer.toString

Float.toJSON = Float.toString

String.toJSON = () => '"' + this.replaceCharacter('\\', '\\\\').replace('"', '\\"') + '"'

Hash.toJSON = func:
	private sb = String.Builder.new()
	sb << "{"
	private it = this.getIterator()
	while it.hasNext()
		private key, value = it.next()
		sb << key.toJSON() << ": " << value.toJSON()
		if it.hasNext()
			sb << ","
		end
	end
	sb << "}"
	sb.toString()
end

Iterable.toJSON = func:
	private sb = String.Builder.new()
	sb << "["
	private it = this.getIterator()
	while it.hasNext()
		sb << it.next().toJSON()
		if it.hasNext()
			sb << ","
		end
	end
	sb << "]"
	sb.toString()
end
