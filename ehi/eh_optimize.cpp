/*
 * eh_optimize
 *
 * Optimize the AST of an EH program.
 */
#include "eh.hpp"
#include "std_lib/Node.hpp"

// Don't think this would actually help; problem is that we need to put the method name in an ehval_p, which costs memory
#define UNARY_OP(op, method) case op: \
 	return eh_addnode(T_CALL_METHOD, optimize(paras[0], context), String::make(method), optimize(paras[1], context))

static inline ehval_p val(Node::t *op) {
	return Node::make(op);
}

ehval_p EHI::optimize(ehval_p node, ehcontext_t context) {
	if(!node->is_a<Node>()) {
		return node;
	}
	Node::t *op = node->get<Node>();
	if(op == nullptr) {
		return nullptr;
	}
	ehval_p *paras = op->paras;
	switch(op->op) {
		case T_LITERAL:
			return paras[0];
		case T_NULL:
			return nullptr;
		case T_GROUPING: {
			ehval_p inner = paras[0];
			if(inner->is_a<Node>() && inner->get<Node>()->op != T_COMMA) {
				return optimize(inner, context);
			} else {
				return val(eh_addnode(T_GROUPING, optimize(inner, context)));
			}
		}
		case T_CALL: {
			// optimize method call
			ehval_p func = paras[0];
			ehval_p args = optimize(paras[1], context);
			if(func->is_a<Node>() && func->get<Node>()->op == T_ACCESS) {
				ehval_p *inner_paras = func->get<Node>()->paras;
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
			return val(eh_addnode(op->op, paras[0], optimize(paras[1], context)));
		default: {
			const int nparas = node_nparas.at(op->op).second;
			Node::t *out = new Node::t(op->op, nparas);
			for(int i = 0; i < nparas; i++) {
				out->paras[i] = optimize(paras[i], context);
			}
			return val(out);
		}
	}
}
