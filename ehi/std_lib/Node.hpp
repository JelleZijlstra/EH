#ifndef EH_NODE_H_
#define EH_NODE_H_

#include "Enum.hpp"
#include "Null.hpp"

class Node : public Enum_Instance::t {
public:
	const static unsigned int node_type_id = 8;

	Node(unsigned int op, unsigned int nparas) : Enum_Instance::t(node_type_id, op, nparas, nullptr) {
		if(nparas > 0) {
			members = new ehval_p[nparas];
		}
	}

	std::string decompile(int level) const;

	static ehval_p make(Node *val, EHInterpreter *parent) {
		if(val == nullptr) {
			return Null::make();
		} else {
			return parent->allocate<Enum_Instance>(val);
		}
	}

	static bool is_a(ehval_p obj) {
		return obj->is_a<Enum_Instance>() && (obj->get<Enum_Instance>()->type_id == node_type_id);
	}
};

Node *eh_addnode(unsigned int opcode);
Node *eh_addnode(unsigned int opcode, ehval_p first);
Node *eh_addnode(unsigned int opcode, ehval_p first, ehval_p second);
Node *eh_addnode(unsigned int opcode, ehval_p first, ehval_p second, ehval_p third);
Node *eh_addnode(unsigned int opcode, ehval_p first, ehval_p second, ehval_p third, ehval_p fourth);

#include "../eh_libclasses.hpp"

EH_ENUM_INITIALIZER(Node);

EH_METHOD(Node, execute);
EH_METHOD(Node, decompile);

EH_CHILD_CLASS(Node, Context) {
public:
	typedef ehcontext_t *type;
	type value;

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->object, value->scope };
	}

	Node_Context(type val) : value(val) {}

	virtual ~Node_Context() {
		delete value;
	}

	virtual void printvar(printvar_set &set, int level, class EHI *ehi) override;

	static ehval_p make(const ehcontext_t &context, EHInterpreter *parent) {
		return parent->allocate<Node_Context>(new ehcontext_t(context));
	}
};

EH_INITIALIZER(Node_Context);

EH_METHOD(Node_Context, operator_colon);
EH_METHOD(Node_Context, getObject);
EH_METHOD(Node_Context, getScope);

#endif /* EH_NODE_H_ */
