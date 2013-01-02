#ifndef EH_ENUM_H_
#define EH_ENUM_H_

#include <vector>

#include "std_lib_includes.hpp"

EH_CHILD_CLASS(Enum, Member) {
public:
	class t {
	public:
		typedef const std::vector<std::string> params_t;

		ehval_p parent_enum;
		const int size;
		const std::string name;
		params_t params;

		t(ehval_p _parent, const std::string &_name) : parent_enum(_parent), size(0), name(_name), params() {}

		t(ehval_p _parent, const std::string &_name, int _size, params_t &_params) : parent_enum(_parent), size(_size), name(_name), params(_params) {
			assert(params.size() == (unsigned long) size);
		}

		std::string toString() const;

		static ehval_p make(ehval_p e, Enum_Member::t *em, EHI *ehi);

		static ehval_p make(ehval_p e, const char *name, EHI *ehi);

		static ehval_p make(ehval_p e, const char *name, params_t &params, EHI *ehi);
	};

	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() {
		return { value->parent_enum };
	}

	~Enum_Member() {
		delete value;
	}

	Enum_Member(type val) : value(val) {}

	static ehval_p make(t *em, EHInterpreter *parent) {
		return parent->allocate<Enum_Member>(em);
	}
};

EH_CLASS(Enum) {
public:
	class t {
	private:
		size_t nmembers;
		std::vector<ehval_p> members;
		const std::string name;

		t(const std::string &_name, ehval_p _contents) : nmembers(0), members(0), name(_name), contents(_contents) {}

		static void add_member(ehval_p e, const char *name, ehval_p member, EHI *ehi);
	public:
		// prototype
		ehval_p contents;

		std::string toString() const;

		size_t size() const {
			return nmembers;
		}

		std::vector<ehval_p> get_members() {
			return members;
		}

		static void add_nullary_member(ehval_p e, const char *name, EHI *ehi);

		static void add_member_with_arguments(ehval_p e, const char *name, Enum_Member::t::params_t params, EHI *ehi);

		static ehval_p make(const char *name, EHI *ehi);

		static t *extract_enum(ehval_p obj);
	};

	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() {
		std::list<ehval_p> out;
		out.push_back(value->contents);
		for(auto &i : value->get_members()) {
			out.push_back(i);
		}
		return out;
	}

	~Enum() {
		delete value;
	}

	Enum(type val) : value(val) {}

	static ehval_p make(t *value, EHInterpreter *parent) {
		return parent->allocate<Enum>(value);
	}
};

EH_CHILD_CLASS(Enum, Instance) {
public:
	class t {
	public:
		typedef std::vector<ehval_p> args_t;

		ehval_p member_ptr;

		const args_t args;

		t(ehval_p _member_ptr, args_t _args) : member_ptr(_member_ptr), args(_args) {}

		std::string toString(EHI *ehi, ehcontext_t context);

		int compare(Enum_Instance::t *rhs, EHI *ehi, ehcontext_t context);

		ehval_p member() {
			return member_ptr;
		}

		ehval_p get(unsigned int i) {
			return args[i];
		}

		static ehval_p make(ehval_p member, args_t args, EHI *ehi);
	};

	typedef t *type;
	type value;

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() {
		std::list<ehval_p> out;
		out.push_back(value->member_ptr);
		for(auto &i : value->args) {
			out.push_back(i);
		}
		return out;
	}

	~Enum_Instance() {
		delete value;
	}

	Enum_Instance(type val) : value(val) {}

	static ehval_p make(type val, EHInterpreter *parent) {
		return parent->allocate<Enum_Instance>(val);
	}
};

EH_INITIALIZER(Enum);

EH_METHOD(Enum, new);
EH_METHOD(Enum, size);
EH_METHOD(Enum, toString);

EH_INITIALIZER(Enum_Member);

EH_METHOD(Enum_Member, new);
EH_METHOD(Enum_Member, operator_colon);
EH_METHOD(Enum_Member, toString);

EH_INITIALIZER(Enum_Instance);

EH_METHOD(Enum_Instance, toString);
EH_METHOD(Enum_Instance, compare);

#endif /* EH_ENUM_H_ */
