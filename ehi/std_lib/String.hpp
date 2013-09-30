#include "std_lib_includes.hpp"

#ifndef EH_STRING_H_
#define EH_STRING_H_
/*
 * String class
 */

EH_CLASS(String) {
public:
	typedef const char *type;
	type value;

	virtual bool belongs_in_gc() const {
		return false;
	}

	virtual std::string decompile(int level) const override {
		return std::string("\"") + value + "\"";
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		std::cout << "@string \"" << value << "\"" << std::endl;
	}

	static ehval_p make(const char *string) {
		return static_cast<ehval_t *>(new String(string));
	}

	virtual ~String() {
		delete[] value;
	}
private:
	String(type c) : value(c) {}
};

EH_METHOD(String, operator_colon);
EH_METHOD(String, length);
EH_METHOD(String, operator_arrow);
EH_METHOD(String, operator_plus);
EH_METHOD(String, compare);
EH_METHOD(String, operator_equals);
EH_METHOD(String, toString);
EH_METHOD(String, toInteger);
EH_METHOD(String, toFloat);
EH_METHOD(String, toBool);
EH_METHOD(String, toRange);
EH_METHOD(String, charAtPosition);
EH_METHOD(String, getIterator);
EH_METHOD(String, trim);
EH_METHOD(String, replace);
EH_METHOD(String, doesMatch);

EH_INITIALIZER(String);

EH_CHILD_CLASS(String, Iterator) {
private:
	class t {
	public:
		t(ehval_p _string) : string(_string), position(0) {}
		~t() {}
		bool has_next() const;
		char next();
		char peek() const;
		ehval_p string;
	private:
		size_t position;
		t(const t&);
		t operator=(const t&);
	};

public:
	String_Iterator(t *val) : value(val) {}

	typedef t *type;
	type value;

	static ehval_p make(ehval_p string, EHInterpreter *parent);

	virtual bool belongs_in_gc() const {
		return true;
	}

	virtual std::list<ehval_p> children() const override {
		return { value->string };
	}

	virtual ~String_Iterator() {}
};
EH_METHOD(String_Iterator, operator_colon);
EH_METHOD(String_Iterator, hasNext);
EH_METHOD(String_Iterator, next);
EH_METHOD(String_Iterator, peek);

EH_INITIALIZER(String_Iterator);

EH_CHILD_CLASS(String, Builder) {
private:
	class t {
	private:
		std::ostringstream strings;

	public:
		t() : strings() {}

		void add_string(const char *item) {
			strings << item;
		}

		const std::string get_string() {
			return strings.str();
		}
	};

public:
	typedef t *type;
	type value;

	static ehval_p make() {
		return static_cast<ehval_t *>(new String_Builder());
	}

	String_Builder() : value(new t()) {}

	virtual bool belongs_in_gc() const {
		return false;
	}

	virtual ~String_Builder() {
		delete value;
	}

	virtual void printvar(printvar_set &set, int level, EHI *ehi) override {
		std::cout << "@string builder \"" << value->get_string() << "\"" << std::endl;
	}
};

EH_METHOD(String_Builder, operator_colon);
EH_METHOD(String_Builder, operator_leftshift);
EH_METHOD(String_Builder, toString);

EH_INITIALIZER(String_Builder);

#endif /* EH_STRING_H_ */
