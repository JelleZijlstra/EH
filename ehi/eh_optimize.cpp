/*
 * eh_optimize
 *
 * Optimize the AST of an EH program.
 */
#include "eh.hpp"
#include "eh_tree.hpp"

// Don't think this would actually help; problem is that we need to put the method name in an ehretval_p, which costs memory
#define UNARY_OP(op, method) case op: \
 	return eh_addnode(T_CALL_METHOD, optimize(paras[0], context), ehretval_t::make_string(method), optimize(paras[1], context))

static inline ehretval_p val(opnode_t *op) {
	return ehretval_t::make(op);
}

ehretval_p EHI::optimize(ehretval_p node, ehcontext_t context) {
	if(node->type() != op_e) {
		return node;
	}
	opnode_t *op = node->get_opval();
	if(op == NULL) {
		return NULL;
	}
	ehretval_p *paras = op->paras;
	const int nparas = op->nparas;
	switch(op->op) {
		case T_LITERAL:
			return paras[0];
		case T_NULL:
			return NULL;
		case '(': {
			ehretval_p inner = paras[0];
			if(inner->type() == op_e && inner->get_opval()->op != ',') {
				return optimize(inner, context);
			} else {
				return val(eh_addnode('(', optimize(inner, context)));
			}
		}
		case ':': {
			// optimize method call
			ehretval_p func = paras[0];
			ehretval_p args = optimize(paras[1], context);
			if(func->type() == op_e && func->get_opval()->op == '.') {
				ehretval_p *inner_paras = func->get_opval()->paras;
				ehretval_p base = optimize(inner_paras[0], context);
				ehretval_p method = optimize(inner_paras[1], context);
				return val(eh_addnode(T_CALL_METHOD, base, method, args));
			} else {
				ehretval_p optimized_func = optimize(func, context);
				return val(eh_addnode(':', optimized_func, args));
			}
		}
		case '=':
		case T_FUNC:
			// don't optimize the lvalue
			return val(eh_addnode(op->op, paras[0], optimize(paras[1], context)));
		default: {
			opnode_t *out = opnode_t::make(op->op, nparas);
			for(int i = 0; i < nparas; i++) {
				out->paras[i] = optimize(paras[i], context);
			}
			return val(out);
		}
	}
}
