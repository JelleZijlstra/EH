#!/usr/bin/ehi
include '../lib/library.eh'

private f = x => x * x
assert(f.isA Function, "Function literal")

# @method operator()
assert(f.operator() 3 == 9, "calling function.operator() directly")

# @method decompile
assert(f.decompile() == "func: x
	x.operator*(x)
end", "decompile never generates a short function literal")

# @method toString
assert(f.toString() == "x => (user code)")
assert(echo.toString() == "(args) => (native code)")

# @method bindTo
private f2 = () => this.toString()
assert(f2() == "(global execution context)", "executed in global context")
private f2_bound = f2.bindTo 3
assert(f2_bound() == '3', "executed in context of integer")

# @method args
assert(f.args() == Node.T_VARIABLE("x"), "argument is x")
private f2 _ = 3
assert(f2.args() == Node.T_ANYTHING, "argument is ignored")

# @method code
assert(f.code() == Node.T_CALL_METHOD(Node.T_VARIABLE("x"), "operator*", Node.T_VARIABLE("x")), "code is squaring")
assert(f2.code() == Node.T_LITERAL(3), "body is just 3")
