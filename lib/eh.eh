# Enhancements to the EH module

# Eval as an expression
const EH.exprEval = func: expr
	private obj = EH.eval("x = (" + expr + ");")
	obj.x
end

# Eval as either an expression or a statement
const EH.universalEval = func: input
	try
		EH.eval input
	catch
		EH.exprEval input
	end
end

const EH.parseFile = file => EH.parse(File.readFile file)

const EH.equalType = lhs, rhs => lhs.typeId() == rhs.typeId()

Object.equalObject = rhs => Object.toString.apply(this, ()) == Object.toString.apply(rhs, ())

# Escape shell arguments
EH.escapeShellArgument = string => "'" + string.replaceCharacter("'", "\\'") + "'"
