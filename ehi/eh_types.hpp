#include <map>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define EH_CLASS_INHERIT(cname, inherited) class cname; \
	template<> inline const char *ehval_t::name<cname>() { return #cname; } \
	class cname : public inherited

#define EH_CLASS(cname) EH_CLASS_INHERIT(cname, ehval_t)

// attributes of class members
enum visibility_enum {
	public_e = 0,
	private_e = 1
};

enum static_enum {
	nonstatic_e = 0,
	static_e = 1
};

enum const_enum {
	nonconst_e = 0,
	const_e = 1
};

// struct for class member attributes
struct attributes_t {
	visibility_enum visibility : 2;
	static_enum isstatic : 1;
	const_enum isconst : 1;

	// convenience methods
	constexpr static attributes_t make_const() {
		return attributes_t(public_e, nonstatic_e, const_e);
	}
	constexpr static attributes_t make_static() {
		return attributes_t(public_e, static_e, nonconst_e);
	}
	constexpr static attributes_t make_private() {
		return attributes_t(private_e, nonstatic_e, nonconst_e);
	}

	constexpr attributes_t(visibility_enum v = public_e, static_enum s = nonstatic_e, const_enum c = nonconst_e) : visibility(v), isstatic(s), isconst(c) {}
};

template<class T>
static void add_tabs(T &out, int levels) {
	for(int i = 0; i < levels; i++) {
		out << "\t";
	}
}

typedef std::unordered_set<void *> printvar_set;

class ehval_t : public garbage_collector<ehval_t>::data {
public:
	typedef garbage_collector<ehval_t>::strong_pointer ehval_p;
	typedef garbage_collector<ehval_t>::weak_pointer ehval_w;

	// context
	struct ehcontext_t {
		ehval_p object;
		ehval_p scope;

		ehcontext_t(ehval_p _object, ehval_p _scope) : object(_object), scope(_scope) {}

		ehcontext_t(ehval_p in) : object(in), scope(in) {}

		ehcontext_t() : object(nullptr), scope(nullptr) {}
	};

	// Variables and object members (which are the same)
	class ehmember_t {
	public:
		typedef refcount_ptr<ehmember_t> ehmember_p;

		attributes_t attribute;
		ehval_w value;

		// destructor
		~ehmember_t() {}

		ehmember_t() : attribute(attributes_t()), value(nullptr) {}
		ehmember_t(attributes_t atts) : attribute(atts), value(nullptr) {}
		ehmember_t(attributes_t atts, ehval_p val) : attribute(atts), value(val) {}

		// convenience methods
		bool isstatic() const {
			return this->attribute.isstatic == static_e;
		}
		bool isconst() const {
			return this->attribute.isconst == const_e;
		}
		bool isprivate() const {
			return this->attribute.visibility == private_e;
		}

		static ehmember_p make(attributes_t attribute, ehval_p value) {
			ehmember_p out(attribute, value);
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

	virtual void printvar(printvar_set &set, int level, class EHI *ehi);

	template<class T>
	typename T::type get() const {
		static_assert(sizeof(T) <= sizeof(garbage_collector<ehval_t>::block), "Type derived from ehval_t must fit in GC");
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
	static inline const char *name() {
		// should never happen; classes will provide specializations
		throw;
	}

	static ehval_p null_object();

	/*
	 * Comparison
	 */
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

	void check_can_set_member(ehmember_p current_member, const char *name, ehcontext_t context, class EHI *ehi);
	/*
	 * Property access
	 */

	// set this member directly on the current object, without worrying about inheritance
	void set_member(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);

	// set this property on this object, first looking for the property among inherited scopes
	ehmember_p set_property(const char *name, ehval_p value, ehcontext_t context, class EHI *ehi);

	// set this member directly on the current object, without worrying about inheritance
	void set_instance_member(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);

	// set this property on this object, first looking for the property among inherited scopes
	ehmember_p set_instance_property(const char *name, ehval_p value, ehcontext_t context, class EHI *ehi);

	// get the property of this name, creating a Binding for methods if necessary
	ehval_p get_property(const char *name, ehcontext_t context, class EHI *ehi);

	// same, but does not create a Binding
	ehmember_p get_property_no_binding(const char *name, ehcontext_t context, class EHI *ehi);

	// similar, but looks up the scope chain
	ehmember_p get_property_up_scope_chain(const char *name, ehcontext_t context, class EHI *ehi);

	// get a member on a class's prototype, throwing a NameError if it's not found
    ehval_p get_instance_member_throwing(const char *name, ehcontext_t context, class EHI *ehi);

	// get a member on a class's prototype
    virtual ehmember_p get_instance_member(const char *name, ehcontext_t context, class EHI *ehi, bool include_object = true);

	// get the property of this name, only looking at the current object
	virtual ehmember_p get_instance_member_current_object(const char *name, ehcontext_t context, class EHI *ehi);

	// get the underlying type object for this value
	virtual ehval_p get_type_object(const class EHInterpreter *parent);

	virtual unsigned int get_type_id(const class EHInterpreter *parent);

	// get the property of this name, only looking at the current object
	virtual ehmember_p get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi);

	// set this member directly, with no further checks
	virtual void set_member_directly(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);

	// set this member directly, with no further checks
	virtual void set_instance_member_directly(const char *name, ehmember_p value, ehcontext_t context, class EHI *ehi);

	virtual const std::list<ehval_w> get_super_classes() {
		return std::list<ehval_w>();
	}

	virtual void inherit(ehval_p superclass) {
		// should be defined whenever has_instance_members returns true
		assert(!has_instance_members());
	}

	virtual bool inherits(ehval_p superclass) {
		return false;
	}

    virtual std::set<std::string> member_set(const EHInterpreter *interpreter_parent);

    virtual std::set<std::string> instance_member_set(const EHInterpreter *interpreter_parent) {
    	return {};
    }

	// returns the parent scope of the current object
	virtual ehval_p get_parent_scope() {
		return nullptr;
	}

	// visibility
	virtual bool can_access_private(ehcontext_t context, class EHI *ehi);

	/*
	 * Permissions
	 */
	virtual bool has_instance_members() const {
		return false;
	}
	virtual bool can_create_property() {
		return false;
	}
	virtual bool can_set_property() {
		return false;
	}
};

typedef ehval_t::ehval_p ehval_p;
typedef ehval_t::ehval_w ehval_w;

typedef array_ptr<ehval_p> ehretval_a;

typedef ehval_t::ehcontext_t ehcontext_t;
typedef ehval_t::ehmember_t ehmember_t;

typedef ehval_p (*ehlibmethod_t)(ehval_p, ehval_p, class EHI *);

typedef ehmember_t::ehmember_p ehmember_p;

class eh_exception : public std::exception {
public:
	ehval_p content;

	eh_exception(ehval_p _content) : content(_content) {}

	virtual ~eh_exception() throw();
};

inline int ehval_t::naive_compare(const ehval_p &rhs) const {
	// just compare pointer values
	const char *lhs_val = reinterpret_cast<const char *>(this);
	const char *rhs_val = reinterpret_cast<const char *>(rhs.operator->());
	if(lhs_val < rhs_val) {
		return -1;
	} else if(lhs_val == rhs_val) {
		return 0;
	} else {
		return 1;
	}
}
