class BinaryTreeSet
	# A tree consists of Nodes with a value in the middle and either null or a Node at the left and right
	class BTNode
		private n

		const initialize = l, val, r => (this.n = [l, val, r])

		const insert = func: val
			private const insertHelper = func: node, val
				if node == null
					BTNode(null, val, null)
				else
					node.insert val
					node
				end
			end
			private const comparison = val <=> this.n->1
			if comparison < 0
				this.n->0 = insertHelper(this.n->0, val)
			elsif comparison == 0
				# do nothing
			else
				this.n->2 = insertHelper(this.n->2, val)
			end
		end

		const reduce = func: firstVal, f
			private tmp = match this.n->0
				case null; firstVal
				case _; this.n->0.reduce(firstVal, f)
			end
			tmp = f(tmp, this.n->1)
			match this.n->2
				case null; tmp
				case _; this.n->2.reduce(tmp, f)
			end
		end

		const has = func: elt
			private helper = func: node, elt
				if node == null
					false
				else
					node.has elt
				end
			end
			private comparison = elt <=> this.n->1
			if comparison < 0
				helper(this.n->0, elt)
			elsif comparison == 0
				true
			else
				helper(this.n->2, elt)
			end
		end

		const toString = () => '(' + this.n->0 + ',' + this.n->1 + ',' + this.n->2 + ')'
	end
	private Node = Node

	private base = null

	const add elt = match this.base
		case null; this.base = BTNode(null, elt, null); null
		case _; this.base.insert elt; null
	end

	const has = elt => match this.base
		case null; false
		case _; this.base.has elt
	end

	const reduce = firstVal, f => match this.base
		case null; firstVal
		case _; this.base.reduce(firstVal, f)
	end

	const toString() = '{' + this.reduce("", func: accum, val
		val.toString() + ', ' + accum
	end) + '}'

	const debug() = this.base.toString()
end
