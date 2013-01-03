#ifndef EH_NODE_H_
#define EH_NODE_H_

extern const std::map<int, std::pair<const char *, int> > node_nparas;

EH_CLASS(Node) {
public:
	class t {
	public:
		int op; // Type of operator
		ehval_p *paras; // Parameters

		~t() {
			delete[] paras;
		}

		t(int _op, int nparas) : op(_op) {
			if(nparas > 0) {
				paras = new ehval_p[nparas];
			} else {
				paras = nullptr;
			}
		}
	};

	typedef t *type;
	type value;

	~Node() {
		delete value;
	}

	Node(type val) : value(val) {}

	virtual bool belongs_in_gc() const {
		return false;
	}

	virtual std::string decompile(int level);

	static ehval_p make(type val) {
		return static_cast<ehval_t *>(new Node(val));
	}
};

Node::t *eh_addnode(int opcode);
Node::t *eh_addnode(int opcode, ehval_p first);
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second);
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third);
Node::t *eh_addnode(int opcode, ehval_p first, ehval_p second, ehval_p third, ehval_p fourth);

#endif /* EH_NODE_H_ */
