#ifndef EH_NODE_H_
#define EH_NODE_H_

#include "Enum.hpp"
#include "Null.hpp"

class Node : public Enum_Instance::t {
public:
	const static int node_id = 6;

	Node(int op, int nparas) : Enum_Instance::t(node_id, op, nparas, nullptr) {
		if(nparas > 0) {
			members = new ehval_p[nparas];
		}
	}

	virtual std::string decompile(int level);

	static ehval_p make(Node *val, EHInterpreter *parent) {
		if(val == nullptr) {
			return Null::make();
		} else {
			return parent->allocate<Enum_Instance>(val);
		}
	}

	static bool is_a(ehval_p obj) {
		return obj->is_a<Enum_Instance>() && (obj->get<Enum_Instance>()->type_id == node_id);
	}
};

Node *eh_addnode(int opcode);
Node *eh_addnode(int opcode, ehval_p first);
Node *eh_addnode(int opcode, ehval_p first, ehval_p second);
Node *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third);
Node *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third, ehval_p fourth);

#include "../eh_libclasses.hpp"

EH_INITIALIZER(Node);

EH_METHOD(Node, execute);
EH_METHOD(Node, decompile);

EH_CHILD_CLASS(Node, Context) {
public:
	typedef ehcontext_t *type;
	type value;

	bool belongs_in_gc() const {
		return true;
	}

	std::list<ehval_p> children() {
		return { value->object, value->scope };
	}

	Node_Context(type val) : value(val) {}

	~Node_Context() {
		delete value;
	}

	virtual void printvar(printvar_set &set, int level, class EHI *ehi);

	static ehval_p make(const ehcontext_t &context, EHInterpreter *parent) {
		return parent->allocate<Node_Context>(new ehcontext_t(context));
	}
};

EH_INITIALIZER(Node_Context);

EH_METHOD(Node_Context, initialize);
EH_METHOD(Node_Context, object);
EH_METHOD(Node_Context, scope);

#endif /* EH_NODE_H_ */
