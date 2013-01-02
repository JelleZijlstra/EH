// Operator
EH_CLASS(Node) {
public:
	class t {
	public:
		int op; // Type of operator
		int nparas; // Number of parameters
		ehval_p *paras; // Parameters

		~t() {
			if(nparas > 0) {
				delete[] paras;
			}
		}

		t(int _op, int _nparas) : op(_op), nparas(_nparas) {
			if(nparas > 0) {
				paras = new ehval_p[nparas];
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
