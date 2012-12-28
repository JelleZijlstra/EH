#!/usr/bin/ehi

class AssertionFailure
	this.inherit Exception

	private desc

	public initialize = this.desc => ()

	public toString = () => this.desc
end

public assert = func: expr
	if expr.isA Tuple
		expr, desc = expr
	else
		desc = "(no description)"
	end
	if !(expr.isA Bool)
		throw(AssertionFailure.new "assert expression must be a Bool")
	end
	if !expr
		throw(AssertionFailure.new desc)
	end
end

public assertThrows = func: args
	if args.length() > 2
		expr, exceptionClass, desc = args
	else
		expr, exceptionClass = args
		desc = "(no description)"
	end
	try
		expr()
		throw(AssertionFailure.new(desc))
	catch if exception.isA exceptionClass
	end
end
