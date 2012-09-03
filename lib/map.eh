class BinaryTreeMap
	# A tree consists of Nodes with a value in the middle and either null or a Node at the left and right
	class Node
		private n

		const initialize = func: l, key, val, r -> (this.n = [l, key, val, r])

		const insert = func: key, val
			insertHelper = func: node, key, val
				if node == null
					Node.new null, key, val, null
				else
					node.insert val
					node
				end
			end
			private comparison = key <=> this.n->1
			if comparison < 0
				this.n->0 = insertHelper this.n->0, key, val
			else
				if comparison == 0
					this.n->2 = val
				else
					this.n->3 = insertHelper this.n->3, key, val
				end
			end
		end

		const reduce = func: firstVal, f
			tmp = given this.n->0
				case null; firstVal
				default; this.n->0.reduce firstVal, f
			end
			tmp = f tmp, this.n->1, this.n->2
			given this.n->3
				case null; tmp
				default; this.n->3.reduce tmp, f
			end
		end

		const get = func: key
			helper = func: node, key -> given node
				case null; null
				default; node.get key
			end
			private comparison = key <=> this.n->1
			if comparison < 0
				helper this.n->0, key
			else
				if comparison == 0
					this.n->2
				else
					helper this.n->3, key
				end
			end
		end

		const toString = func: -> '(' + (this.n->0) + ',' + (this.n->1) + " => " + (this.n->2) + ',' + (this.n->3) + ')'
	end
	private Node = Node

	private base = null

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

	const operator-> = get
	const operator->= = add

	const has = func: key -> (get key == null)

	const toString = func: -> ('{' + (reduce "", func: accum, key, val -> ((key.toString()) + ' => ' + (val.toString()) + ', ' + accum)) + '}')

	const debug = func: -> (base.toString())
end
