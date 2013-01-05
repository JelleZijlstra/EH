# Useful functions for EH macros

Node.Context.toString = () => "(context: " + this.getObject() + ", " + this.getScope() + ")"

class Macro
	public decorate = func: f, (code, context)
		private processedCode = f code
		processedCode.execute context
	end

	private privateAttrs = Node.T_ATTRIBUTE(Attribute.privateAttribute, Node.T_END)

	public privatize = code => match code
		case Node.T_ASSIGN(@lvalue, @rvalue)
			Node.T_ASSIGN(Node.T_CLASS_MEMBER(privateAttrs, lvalue), rvalue)
	end

	public identity = code => code
end
