class BinaryTreeSet
	# A tree consists of Nodes with a value in the middle and either null or a Node at the left and right
	class Node
		const initialize = func: l, val, r -> [l, val, r]

		const insert = func: val
			private insertHelper = func: node, val
				if node == null
					Node.new null, val, null
				else
					node.insert val
					node
				end
			end
			private comparison = val <=> self->1
			if comparison < 0
				self->0 = insertHelper self->0, val
			else
				if comparison == 0
					# do nothing
				else
					self->2 = insertHelper self->2, val
				end
			end
		end

		const reduce = func: firstVal, f
			private tmp = given self->0
				case null; firstVal
				default; self->0.reduce firstVal, f
			end
			tmp = f tmp, self->1
			given self->2
				case null; tmp
				default; self->2.reduce tmp, f
			end
		end

		const has = func: elt
			private helper = func: node, elt -> (if node == null; false; else node.has elt; end)
			private comparison = elt <=> self->1
			if comparison < 0
				helper self->0, elt
			else
				if comparison == 0
					true
				else
					helper self->2, elt
				end
			end
		end

		const toString = func: -> '(' + (self->0) + ',' + (self->1) + ',' + (self->2) + ')'
	end
	private Node = Node

	private base = null

	const initialize = func: -> null

	const add = func: elt -> given base
		case null; base = Node.new null, elt, null; null
		default; base.insert elt; null
	end

	const has = func: elt -> given base
		case null; false
		default; base.has elt
	end

	const reduce = func: firstVal, f -> given base
		case null; firstVal
		default; base.reduce firstVal, f
	end

	const toString = func: -> ('{' + (reduce "", func: accum, val -> ((val.toString()) + ', ' + accum)) + '}')

	const debug = func: -> (base.toString())
end
