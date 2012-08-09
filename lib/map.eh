class BinaryTreeMap
	# A tree consists of Nodes with a value in the middle and either null or a Node at the left and right
	class Node
		const initialize = func: l, key, val, r -> [l, key, val, r]

		const insert = func: key, val
			insertHelper = func: node, key, val
				if node == null
					Node.new null, key, val, null
				else
					node.insert val
					node
				end
			end
			private comparison = key <=> self->1
			if comparison < 0
				self->0 = insertHelper self->0, key, val
			else
				if comparison == 0
					self->2 = val
				else
					self->3 = insertHelper self->3, key, val
				end
			end
		end

		const reduce = func: firstVal, f
			tmp = given self->0
				case null; firstVal
				default; self->0.reduce firstVal, f
			end
			tmp = f tmp, self->1, self->2
			given self->3
				case null; tmp
				default; self->3.reduce tmp, f
			end
		end

		const get = func: key
			helper = func: node, key -> given node
				case null; null
				default; node.get key
			end
			private comparison = key <=> self->1
			if comparison < 0
				helper self->0, key
			else
				if comparison == 0
					self->2
				else
					helper self->3, key
				end
			end
		end

		const toString = func: -> '(' + (self->0) + ',' + (self->1) + " => " + (self->2) + ',' + (self->3) + ')'
	end
	private Node = Node

	private base = null

	const initialize = func: -> null

	const add = func: key, val -> given base
		case null; base = Node.new null, key, val, null; null
		default; base.insert key, val; null
	end

	const reduce = func: firstVal, f -> given base
		case null; firstVal
		default; base.reduce firstVal, f
	end

	const get = func: key -> given base
		case null; null
		default; base.get key
	end

	const operator_arrow = get
	const operator_arrow_equals = add

	const has = func: key -> (get key == null)

	const toString = func: -> ('{' + (reduce "", func: accum, key, val -> ((key.toString()) + ' => ' + (val.toString()) + ', ' + accum)) + '}')

	const debug = func: -> (base.toString())
end
