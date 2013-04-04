# Enhancements to the EH module

# Eval as an expression
const EH.exprEval expr = EH.eval("x = (" + expr + ");").x

# Eval as either an expression or a statement
const EH.universalEval input = try
	EH.eval input
catch
	EH.exprEval input
end

const EH.parseFile file = EH.parse(File.readFile file)

const EH.equalType(lhs, rhs) = lhs.typeId() == rhs.typeId()

Object.equalObject rhs = Object.toString.apply(this, ()) == Object.toString.apply(rhs, ())

# Escape shell arguments
EH.escapeShellArgument string = "'" + string.replaceCharacter("'", "\\'") + "'"
