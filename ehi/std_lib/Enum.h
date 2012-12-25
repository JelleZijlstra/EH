#ifndef EH_ENUM_H_
#define EH_ENUM_H_

#include <vector>

#include "std_lib_includes.h"

class Enum_Member : public LibraryBaseClass {
public:
	typedef const std::vector<std::string> params_t;

	ehretval_p parent_enum;
	const int size;
	const std::string name;
	params_t params;

	Enum_Member(ehretval_p _parent, const std::string &_name) : parent_enum(_parent), size(0), name(_name), params() {}

	Enum_Member(ehretval_p _parent, const std::string &_name, int _size, params_t &_params) : parent_enum(_parent), size(_size), name(_name), params(_params) {
		assert(params.size() == (unsigned long) size);
	}

	std::string to_string() const;

	static ehretval_p make(ehretval_p e, Enum_Member *em, EHI *ehi);

	static ehretval_p make(ehretval_p e, const char *name, EHI *ehi);

	static ehretval_p make(ehretval_p e, const char *name, params_t &params, EHI *ehi);
};

class Enum : public LibraryBaseClass {
private:
	size_t nmembers;
	std::vector<ehretval_p> members;
	const std::string name;


	Enum(const std::string &_name, ehretval_p _contents) : nmembers(0), members(0), name(_name), contents(_contents) {}

	static void add_member(ehretval_p e, const char *name, ehretval_p member, EHI *ehi);
public:
	ehretval_p contents;

	std::string to_string() const;

	size_t size() const {
		return nmembers;
	}

	static void add_nullary_member(ehretval_p e, const char *name, EHI *ehi);

	static void add_member_with_arguments(ehretval_p e, const char *name, Enum_Member::params_t params, EHI *ehi);

	static ehretval_p make(const char *name, EHI *ehi);

	static Enum *extract_enum(ehretval_p obj);
};

class Enum_Instance : public LibraryBaseClass {
public:
	typedef std::vector<ehretval_p> args_t;
private:
	ehretval_p member_ptr;

	const args_t args;

public:
	Enum_Instance(ehretval_p _member_ptr, args_t _args) : member_ptr(_member_ptr), args(_args) {}

	std::string to_string(EHI *ehi, ehcontext_t context);

	int compare(Enum_Instance *rhs, EHI *ehi, ehcontext_t context);

	ehretval_p member() {
		return member_ptr;
	}

	ehretval_p get(unsigned int i) {
		return args[i];
	}

	static ehretval_p make(ehretval_p member, args_t args, EHI *ehi);
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
