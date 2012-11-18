# Functions related to eval

# Eval as an expression
const exprEval = func: expr
	eval("x = (" + expr + ");")
	x
end

# Eval as either an expression or a statement
const universalEval = func: input
	try
		eval input
	catch
		exprEval input
	end
end


