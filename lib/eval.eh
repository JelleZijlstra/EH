# Functions related to eval

# Eval as an expression
const exprEval = func: expr
	private obj = EH.eval("x = (" + expr + ");")
	obj.x
end

# Eval as either an expression or a statement
const universalEval = func: input
	try
		EH.eval input
	catch
		exprEval input
	end
end


