#include <set>
#include <vector>
#include <unordered_map>

#define EH_CLASS(cname) class cname; \
	template<> inline const char *ehval_t::name<cname>() { return #cname; } \
	class cname : public ehval_t

typedef std::unordered_set<void *> printvar_set;

class ehval_t : public garbage_collector<ehval_t>::data {
public:
	typedef garbage_collector<ehval_t>::pointer ehval_p;

	// context
	struct ehcontext_t {
		ehval_p object;
		ehval_p scope;

		ehcontext_t(ehval_p _object, ehval_p _scope) : object(_object), scope(_scope) {}

		ehcontext_t(ehval_p in) : object(in), scope(in) {}

		ehcontext_t() : object(), scope() {}
	};

	// Variables and object members (which are the same)
	class ehmember_t {
	public:
		typedef refcount_ptr<ehmember_t> ehmember_p;

		attributes_t attribute;
		ehval_p value;

		// destructor
		~ehmember_t() {}

		ehmember_t() : attribute(attributes_t::make()), value() {}
		ehmember_t(attributes_t atts) : attribute(atts), value() {}

		// convenience methods
		bool isstatic() const {
			return this->attribute.isstatic == static_e;
		}
		bool isconst() const {
			return this->attribute.isconst == const_e;
		}

		static ehmember_p make(attributes_t attribute, ehval_p value) {
			ehmember_p out;
			out->attribute = attribute;
			out->value = value;
			return out;
		}
	};

	typedef ehmember_t::ehmember_p ehmember_p;

	ehval_t() {}

	virtual ~ehval_t() {}

	virtual std::list<ehval_p> children() const {
		return {};
	}

	virtual std::string decompile(int level) const {
		// all types that may appear in the AST should define a specialization
		assert(false);
		return "";
	}

	virtual void printvar(printvar_set &set, int level, class EHI *ehi) {
		std::cout << "@other object" << std::endl;
	}

	template<class T>
	typename T::type get() const {
		assert(typeid(*this) == typeid(T));
		// previous line established that this cast is safe
		auto derived = static_cast<const T *>(this);
		return derived->value;
	}

	template<class T>
	bool is_a() const {
		return (this != nullptr) && (typeid(*this) == typeid(T));
	}

	template<class T>
	void assert_type(const char *method, class EHI *ehi);

	bool equal_type(ehval_p rhs) const {
		return typeid(*this) == typeid(rhs.operator*());
	}

	std::type_index type_index() const {
		return std::type_index(typeid(*this));
	}

	template<class T>
	typename T::type get_deep();

	template<class T>
	typename T::type assert_deep(const char *method, class EHI *ehi);

	template<class T>
	bool deep_is_a() const;

	template<class T>
	static inline const char *name() {
		// should never happen; classes will provide specializations
		throw;
	}

	static ehval_t *null_object();

	template<class T>
	bool inherited_is_a();

	int get_type_id(class EHInterpreter *parent);

	ehval_p data();

	bool operator<(const ehval_p &rhs) const {
		return compare(rhs) == -1;
	}

	bool operator==(const ehval_p &rhs) const {
		return compare(rhs) == 0;
	}

	int compare(const ehval_p &rhs) const {
		ehval_t *rhs_obj = rhs.operator->();
		if(this == nullptr) {
			return rhs_obj == nullptr ? 0 : -1;
		} else if(rhs_obj == nullptr) {
			return 1;
		}
		std::type_index l_index(typeid(*this));
		std::type_index r_index(typeid(*rhs_obj));
		if(l_index < r_index) {
			return -1;
		} else if(l_index == r_index) {
			return naive_compare(rhs);
		} else {
			return 1;
		}
	}

	// Compare two ehval_ps (guaranteed to be of the same type)
	int naive_compare(const ehval_p &rhs) const;

	ehmember_p set_member(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);
	ehval_p get_property(const char *name, ehcontext_t context, class EHI *ehi);
	ehval_p get_property_no_binding(const char *name, ehcontext_t context, class EHI *ehi);
	ehmember_p set_property(const char *name, ehval_p value, ehcontext_t context, class EHI *ehi);
	ehval_p get_underlying_object(class EHInterpreter *parent);
};

typedef ehval_t::ehval_p ehval_p;

typedef array_ptr<ehval_p> ehretval_a;

typedef ehval_t::ehcontext_t ehcontext_t;
typedef ehval_t::ehmember_t ehmember_t;

typedef ehval_p (*ehlibmethod_t)(ehval_p, ehval_p, class EHI *);

typedef ehmember_t::ehmember_p ehmember_p;

// EH object
class ehobj_t {
public:
	/*
	 * typedefs
	 */
	typedef std::map<const std::string, ehmember_p> obj_map;
	typedef void (*initializer)(ehobj_t *obj, EHInterpreter *parent);

	/*
	 * properties
	 */
	obj_map members;
	// the object's state data
	ehval_p object_data;
	// the type
	unsigned int type_id;
	// for scoping
	ehval_p parent;
	// inheritance
	std::list<ehval_p> super;

	/*
	 * constructor etcetera
	 */
	ehobj_t() : members(), object_data(), type_id(0), parent(), super() {}
	~ehobj_t();

	// ehobj_t should never be handled directly
	ehobj_t(const ehobj_t&) = delete;
	ehobj_t operator=(const ehobj_t&) = delete;

	/*
	 * const methods
	 */
	ehmember_p get_recursive(const char *name, const ehcontext_t context) const;
	bool context_compare(const ehcontext_t &key, const EHI *ehi) const;

	bool inherited_has(const std::string &key, const EHInterpreter *parent) const;
	ehmember_p inherited_get(const std::string &key, const EHInterpreter *parent) const;
	ehmember_p recursive_inherited_get(const std::string &key) const;

	std::set<std::string> member_set(const EHInterpreter *parent) const;
	ehobj_t *get_parent() const;
	bool inherits(const ehval_p obj, const EHInterpreter *parent) const;


	// inline methods
	size_t size() const {
		return members.size();
	}

	ehmember_p get_known(const std::string &key) const {
		assert(this->has(key));
		return members.at(key);
	}

	bool has(const std::string &key) const {
		return members.count(key);
	}

	bool has(const char *key) const {
		return has(std::string(key));
	}

	/*
	 * non-const methods
	 */

	void insert(const std::string &name, ehmember_p value) {
		members[name] = value;
	}
	void insert(const char *name, ehmember_p value) {
		const std::string str(name);
		this->insert(str, value);
	}

	void inherit(ehval_p superclass) {
		super.push_front(superclass);
	}

	void register_method(const std::string &name, const ehlibmethod_t method, const attributes_t attributes, EHInterpreter *interpreter_parent);
	void register_value(const std::string &name, ehval_p value, const attributes_t attributes);
	int register_enum_class(const ehobj_t::initializer init_func, const char *name, const attributes_t attributes, EHInterpreter *interpreter_parent);
	void add_enum_member(const char *name, const std::vector<std::string> &params, EHInterpreter *parent, int member_id = 0);
	int register_member_class(const char *name, const ehobj_t::initializer init_func, const attributes_t attributes, EHInterpreter *interpreter_parent);

	template<class T>
	int register_member_class(const ehobj_t::initializer init_func, const char *name, const attributes_t attributes, EHInterpreter *interpreter_parent, ehval_p the_class = nullptr);
};

EH_CLASS(Object) {
public:
	typedef ehobj_t *const type;
	type value;

	Object(ehobj_t *val) : value(val) {}

	virtual ~Object() {
		delete value;
	}

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		for(auto &kv : value->members) {
			out.push_back(kv.second->value);
		}
		out.push_back(value->parent);
		out.push_back(value->object_data);
		for(auto &i : value->super) {
			out.push_back(i);
		}
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override;

	static ehval_p make(ehobj_t *obj, EHInterpreter *parent);

	template<class T>
	bool inherits() const {
		for(const auto &i : value->super) {
			if(i->inherited_is_a<T>()) {
				return true;
			}
		}
		return false;
	}
};

class type_repository {
private:
	class type_info {
	public:
		const bool is_inbuilt;
		const std::string name;
		ehval_p type_object;

		type_info(bool ii, const std::string &n, ehval_p to) : is_inbuilt(ii), name(n), type_object(to) {}
	};

	std::unordered_map<std::type_index, int> inbuilt_types;

	std::vector<type_info> types;

public:
	template<class T>
	int register_inbuilt_class(ehval_p object) {
		const int type_id = register_class(ehval_t::name<T>(), object, true);
		inbuilt_types[std::type_index(typeid(T))] = type_id;
		return type_id;
	}

	template<class T>
	ehval_p get_primitive_class() const {
		return types.at(get_primitive_id<T>()).type_object;
	}

	template<class T>
	int get_primitive_id() const {
		return inbuilt_types.at(std::type_index(typeid(T)));;
	}

	int get_type_id(const ehval_p obj) const {
		return inbuilt_types.at(std::type_index(typeid(*obj.operator->())));
	}

	int register_class(const std::string &name, const ehval_p value, const bool is_inbuilt = false) {
		const int type_id = types.size();
		types.push_back(type_info(is_inbuilt, name, value));
		return type_id;
	}

	const std::string &get_name(const ehval_p obj) const {
		if(obj->is_a<Object>()) {
			return types.at(obj->get<Object>()->type_id).name;
		} else {
			const int type_id = inbuilt_types.at(obj->type_index());
			return types.at(type_id).name;
		}
	}
	ehval_p get_object(const ehval_p obj) const {
		if(obj->is_a<Object>()) {
			return types.at(obj->get<Object>()->type_id).type_object;
		} else {
			const int type_id = inbuilt_types.at(obj->type_index());
			return types.at(type_id).type_object;
		}
	}

	ehval_p get_object(const int type_id) const {
		return types.at(type_id).type_object;
	}

	type_repository() : inbuilt_types(), types() {}
};

class eh_exception : public std::exception {
public:
	ehval_p content;

	eh_exception(ehval_p _content) : content(_content) {}

	virtual ~eh_exception() throw();
};

template<class T>
inline typename T::type ehval_t::get_deep() {
	ehval_t *me = this;
	if(typeid(*this) == typeid(Object)) {
		me = get<Object>()->object_data.operator->();
	}
	return me->get<T>();
}

template<class T>
inline typename T::type ehval_t::assert_deep(const char *method, class EHI *ehi) {
	ehval_t *me = this;
	if(typeid(*me) == typeid(Object)) {
		me = get<Object>()->object_data.operator->();
	}
	me->assert_type<T>(method, ehi);
	return me;
}

template<class T>
inline bool ehval_t::deep_is_a() const {
	if(this->is_a<Object>()) {
		ehval_t *me = get<Object>()->object_data.operator->();
		return me->is_a<T>();
	} else {
		return this->is_a<T>();
	}
}

template<class T>
inline bool ehval_t::inherited_is_a() {
	if(this->is_a<T>()) {
		return true;
	} else if(this->is_a<Object>()) {
		Object *obj = static_cast<Object *>(this);
		return obj->inherits<T>();
	} else {
		return false;
	}
}

inline ehval_p ehval_t::data() {
	if(this->is_a<Object>()) {
		return get<Object>()->object_data;
	} else {
		return this;
	}
}

inline int ehval_t::naive_compare(const ehval_p &rhs) const {
	// just compare pointer values (should "work" even if objects are not Object instances)
	ehval_t *rhs_p = rhs.operator->();
	const char *lhs_val = reinterpret_cast<const char *>(static_cast<const Object *>(this)->value);
	const char *rhs_val = reinterpret_cast<const char *>(static_cast<const Object *>(rhs_p)->value);
	if(lhs_val < rhs_val) {
		return -1;
	} else if(lhs_val == rhs_val) {
		return 0;
	} else {
		return 1;
	}
}
