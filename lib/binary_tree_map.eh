class BinaryTreeMap
	# A tree consists of Nodes with a value in the middle and either null or a Node at the left and right
	class Node
		private n

		const initialize = l, key, val, r => (this.n = [l, key, val, r])

		const insert = func: key, val
			private const insertHelper = func: node, key, val
				if node == null
					Node.new null, key, val, null
				else
					node.insert val
					node
				end
			end
			private const comparison = key <=> this.n->1
			if comparison < 0
				this.n->0 = insertHelper(this.n->0, key, val)
			elsif comparison == 0
				this.n->2 = val
			else
				this.n->3 = insertHelper(this.n->3, key, val)
			end
		end

		const reduce = func: firstVal, f
			private tmp = match this.n->0
				case null; firstVal
				case _; this.n->0.reduce(firstVal, f)
			end
			tmp = f(tmp, this.n->1, this.n->2)
			match this.n->3
				case null; tmp
				case _; this.n->3.reduce(tmp, f)
			end
		end

		const get = func: key
			private const helper = node, key => match node
				case null; null
				case _; node.get key
			end
			private const comparison = key <=> this.n->1
			if comparison < 0
				helper(this.n->0, key)
			elsif comparison == 0
				this.n->2
			else
				helper(this.n->3, key)
			end
		end

		const toString = () => '(' + this.n->0 + ',' + this.n->1 + " => " + this.n->2 + ',' + this.n->3 + ')'
	end
	private Node = Node

	private base = null

	const add = key, val => match base
		case null; base = Node.new(null, key, val, null); null
		case _; base.insert(key, val); null
	end

	const reduce = firstVal, f => match base
		case null; firstVal
		case _; base.reduce(firstVal, f)
	end

	const get = key => match base
		case null; null
		case _; base.get key
	end

	const operator-> = get
	const operator->= = add

	const has = key => (this.get key == null)

	const toString = () => '{' + reduce("", func: accum, key, val
		key.toString() + ' => ' + val.toString() + ', ' + accum
	end) + '}'

	const debug = () => base.toString()
end
