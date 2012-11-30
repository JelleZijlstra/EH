
Map.toString = func:
	out = "{"
	for key, value in this
		out += key.toString() + ": " + value + ", "
	end
	out + "}"
end

Map.with = func: pairs
	map = Map.new()
	for key, value in pairs
		map->key = value
	end
	map
end
