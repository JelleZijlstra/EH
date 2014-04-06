#ifndef EH_ENUM_H_
#define EH_ENUM_H_

#include <unordered_map>

#include "../eh_libclasses.hpp"

#define EH_ENUM_INITIALIZER(name) void ehinit_ ## name (Enum::t *cls, EHInterpreter *parent)

EH_CLASS(Enum) {
public:
	class member_info {
	public:
		std::string name;
		std::vector<std::string> members;

		member_info(const std::string &_name, const std::vector<std::string> &_members) : name(_name), members(_members) {}

		member_info() : name(), members() {}

		const std::string to_string() const;
	};

	class t : public ehclass_t {
	public:
		size_t n_enum_members;

		std::unordered_map<unsigned int, member_info> enum_members;
		const std::string name;

		const std::string to_string() const;

		size_t size() const {
			return n_enum_members;
		}

		t(const std::string &_name) : ehclass_t(_name), n_enum_members(0), enum_members(), name(_name) {}

		void add_enum_member(const char *name, const std::vector<std::string> &params, EHInterpreter *interpreter_parent, unsigned int member_id = 0);
	};

	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual ~Enum() {
		delete value;
	}

	Enum(type val) : value(val) {}

    virtual ehmember_p get_property_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
    	if(value->has(name)) {
    		return value->get_known(name);
    	} else {
    		return nullptr;
    	}
    }

    virtual void set_member_directly(const char *name, ehmember_p new_value, ehcontext_t context, class EHI *ehi) {
    	value->insert(name, new_value);
    }

    virtual ehval_p get_parent_scope() {
    	return value->parent;
    }

    virtual ehmember_p get_instance_member_current_object(const char *name, ehcontext_t context, class EHI *ehi) {
        return value->get_instance_member_current_object(name, context, ehi);
    }

    virtual const std::list<ehval_w> get_super_classes() {
        return value->super;
    }

	static ehval_p make(const std::string name, EHInterpreter *parent) {
		return parent->allocate<Enum>(new t(name));
	}

    virtual void set_instance_member_directly(const char *name, ehmember_p new_value, ehcontext_t context, class EHI *ehi) {
        value->instance_members[name] = new_value;
    }

    virtual bool has_instance_members() const override {
        return true;
    }

    virtual void inherit(ehval_p cls) override {
        value->inherit(cls);
    }

    virtual bool inherits(ehval_p superclass) override {
        return value->inherits(superclass);
    }

    virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
    	return value->printvar(set, level, ehi);
    }

    virtual const std::string get_name() const override {
        return value->name;
    }

    virtual std::list<ehval_p> children() const override {
        return value->children();
    }

	static ehval_p make_enum_class(const char *name, ehval_p scope, EHInterpreter *parent);
};

typedef void (*enum_initializer)(Enum::t *obj, EHInterpreter *parent);

unsigned int register_enum_class(ehobj_t *cls, const enum_initializer init_func, const char *name, const attributes_t attributes, EHInterpreter *interpreter_parent);


EH_CHILD_CLASS(Enum, Instance) {
public:
	class t {
	public:
		const unsigned int type_id;

		const unsigned int member_id;

		const unsigned int nmembers;

		ehval_p *members;

		t(unsigned int type, unsigned int member, unsigned int n, ehval_p *args) : type_id(type), member_id(member), nmembers(n), members(args) {}

		~t() {
			delete[] members;
		}

		int type_compare(Enum_Instance::t *rhs);

		int compare(Enum_Instance::t *rhs, EHI *ehi, ehcontext_t context);

		ehval_p get(unsigned int i) {
			assert(i < nmembers);
			return members[i];
		}

		const ehval_p get_parent_enum(EHI *ehi) const {
			return ehi->get_parent()->repo.get_object(type_id);
		}

		const Enum::member_info &get_member_info(EHI *ehi) const {
			Enum::t *parent_enum = get_parent_enum(ehi)->get<Enum>();
			return parent_enum->enum_members[member_id];
		}

		bool is_constructor() const {
			return nmembers > 0 && members == nullptr;
		}

		const std::string decompile(int level) const;
	private:
		t(const t &) =delete;
		t operator=(const t &) =delete;
	};

	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const override {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		std::list<ehval_p> out;
		if(value->members != nullptr) {
			const unsigned int size = value->nmembers;
			for(unsigned int i = 0; i < size; i++) {
				out.push_back(value->get(i));
			}
		}
		assert(out.size() == ((value->members == nullptr) ? 0 : value->nmembers));
		return out;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override;

	virtual ~Enum_Instance() {
		delete value;
	}

	Enum_Instance(type val) : value(val) {
		assert(value != nullptr);
	}

	static ehval_p make(type val, EHInterpreter *parent) {
		return parent->allocate<Enum_Instance>(val);
	}

	virtual std::string decompile(int level) const override {
		return value->decompile(level);
	}

	virtual unsigned int get_type_id(const class EHInterpreter *parent) {
		return value->type_id;
	}
private:
	Enum_Instance(const Enum_Instance &) = delete;
	Enum_Instance operator=(const Enum_Instance &) = delete;
};

EH_INITIALIZER(Enum);

EH_METHOD(Enum, operator_colon);
EH_METHOD(Enum, toString);
EH_METHOD(Enum, ofNumeric);

EH_INITIALIZER(Enum_Instance);

EH_METHOD(Enum_Instance, operator_colon);
EH_METHOD(Enum_Instance, compare);
EH_METHOD(Enum_Instance, isConstructor);
EH_METHOD(Enum_Instance, operator_arrow);
EH_METHOD(Enum_Instance, map);
EH_METHOD(Enum_Instance, toString);
EH_METHOD(Enum_Instance, numericValue);
EH_METHOD(Enum_Instance, constructor);

#endif /* EH_ENUM_H_ */
