#!/usr/bin/ehi

# Implement stack tracing in EH
(func:
	class CallStack
		private static size = 0
		private static stack = []

		public static push = func: name
			stack->size = name
			size++
		end

		public static pull = func:
			size--
		end

		# can't use standard library methods, because those are stack'ed and therefore confusing
		public static clone = func:
			private out = []
			for i in size
				out->i = stack->i
			end
			out
		end
	end

	private stringifyStack = func: stack
		private i = 0
		private output = ''
		for _, frame in stack
			output += "" + i + ": " + frame + "\n"
			i += 1
		end
		output
	end

	class MatchChecker
		private f

		public initialize = f => (this.f = f)

		public operator== = rhs => this.f rhs
	end

	private isNotNode = MatchChecker.new(n => n.typeId() != Node.typeId())

	# repeated code sections
	private ehStack = Node.T_ACCESS(Node.T_VARIABLE("EH"), "stack")
	private pull = Node.T_CALL_METHOD(ehStack, "pull", ())
	private retAssign = Node.T_CLASS_MEMBER(Node.T_ATTRIBUTE(Attribute.privateAttribute, Node.T_END), Node.T_VARIABLE("_ret"))

	# process a piece of code to apply stack tracing
	private applyStack = code => match code
		case isNotNode
			code
		case Node.T_CALL(@f, @arg)
			private setStack = Node.T_CALL_METHOD(ehStack, "push", f.decompile())
			private call = Node.T_ASSIGN(retAssign, Node.T_CALL(applyStack f, applyStack arg))
			Node.T_SEPARATOR(setStack, Node.T_TRY_FINALLY(call, Node.T_END, pull))
		case Node.T_ASSIGN(@lhs, @code)
			Node.T_ASSIGN(lhs, applyStack code)
		case Node.T_FUNC(@args, @code)
			Node.T_FUNC(args, applyStack code)
		case Node.T_CASE(@lhs, @code)
			Node.T_CASE(lhs, applyStack code)
		case _
			code.map applyStack
	end

	# overwrite include so that it applies stack tracing to included code
	private realInclude = global.include

	private context = Node.Context(this, this)

	global.include = func: name
		if name->0 != '/'
			name = global.workingDir() + "/" + name
		end
		private code = EH.parse(File.readFile name)
		private stacked = applyStack code
		stacked.execute context
	end

	# overwrite Exception.operator() so that it sets the stack in an exception
	Exception.operator() args = do
		private e = (Class.operator().bindTo this) args
		e.stack = global.EH.stack.clone()
		e
	end

	# overwrite Exception.toString
	private realToString = Exception##toString

	Exception##toString = func:
		private data = (realToString.bindTo this)()
		data + "\n Stack trace:\n" + stringifyStack(this.stack)
	end

	# and export the stack into the EH global module
	EH.stack = CallStack
end)()

include 'library.eh'
