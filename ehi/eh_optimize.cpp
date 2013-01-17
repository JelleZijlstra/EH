/*
 * eh_optimize
 *
 * Optimize the AST of an EH program.
 */
#include "eh.hpp"
#include "std_lib/Node.hpp"
#include "std_lib/Attribute.hpp"
#include "eh.bison.hpp"

// Don't think this would actually help; problem is that we need to put the method name in an ehval_p, which costs memory
#define UNARY_OP(op, method) case op: \
 	return eh_addnode(T_CALL_METHOD, optimize(paras[0], context), String::make(method), optimize(paras[1], context))

#define val(op) Node::make(op, parent)

ehval_p EHI::optimize(ehval_p node, ehcontext_t context) {
	if(Node::is_a(node)) {
		return node;
	}
	Enum_Instance::t *op = node->get<Enum_Instance>();
	if(op == nullptr) {
		return nullptr;
	}
	ehval_w *paras = op->members;
	switch(op->member_id) {
		case T_LITERAL:
			return paras[0];
		case T_NULL:
			return nullptr;
		case T_GROUPING: {
			ehval_p inner = paras[0];
			if(inner->is_a<Enum_Instance>() && inner->get<Enum_Instance>()->member_id != T_COMMA) {
				return optimize(inner, context);
			} else {
				return val(eh_addnode(T_GROUPING, optimize(inner, context)));
			}
		}
		case T_CALL: {
			// optimize method call
			ehval_p func = paras[0];
			ehval_p args = optimize(paras[1], context);
			if(func->is_a<Enum_Instance>() && func->get<Enum_Instance>()->member_id == T_ACCESS) {
				ehval_w *inner_paras = func->get<Enum_Instance>()->members;
				ehval_p base = optimize(inner_paras[0], context);
				ehval_p method = optimize(inner_paras[1], context);
				return val(eh_addnode(T_CALL_METHOD, base, method, args));
			} else {
				ehval_p optimized_func = optimize(func, context);
				return val(eh_addnode(T_CALL, optimized_func, args));
			}
		}
		case T_ASSIGN:
		case T_FUNC:
		case T_CASE:
			// don't optimize the lvalue
			return val(eh_addnode(op->member_id, paras[0], optimize(paras[1], context)));
		default: {
			const int nparas = op->nmembers;
			Node *out = new Node(op->member_id, nparas);
			for(int i = 0; i < nparas; i++) {
				out->members[i] = optimize(paras[i], context);
			}
			return val(out);
		}
	}
}
