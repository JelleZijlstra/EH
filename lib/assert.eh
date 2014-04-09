#!/usr/bin/ehi

class AssertionFailure
	this.inherit Exception

	private desc

	public initialize = this.desc => ()

	public toString() = this.desc
end

public assert expr = do
	if expr.isA Tuple
		expr, desc = expr
	else
		desc = "(no description)"
	end
	if !(expr.isA Bool)
		throw(AssertionFailure "assert expression must be a Bool")
	end
	if !expr
		throw(AssertionFailure desc)
	end
end

public assertThrows args = do
	if args.length() > 2
		expr, exceptionClass, desc = args
	else
		expr, exceptionClass = args
		desc = "(no description)"
	end
	try
		expr()
	catch if exception.isA exceptionClass
		return
	end
	throw(AssertionFailure desc)
end
