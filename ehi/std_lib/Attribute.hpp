#ifndef EH_ATTRIBUTE_H_
#define EH_ATTRIBUTE_H_

#include "Enum.hpp"

// and accompanying enum used by the parser
class Attribute : Enum_Instance::t {
public:
	const static int attribute_type_id = 5;

	enum attribute_enum {
		publica_e,
		privatea_e,
		statica_e,
		consta_e
	};

	static std::map<attribute_enum, ehval_p> instances;

	Attribute(attribute_enum v) : Enum_Instance::t(attribute_type_id, v, 0, nullptr) {}

	static ehval_p make(attribute_enum v, EHInterpreter *parent) {
		if(instances.count(v) > 0) {
			return instances[v];
		} else {
			ehval_p instance = parent->allocate<Enum_Instance>(new Attribute(v));
			instances[v] = instance;
			return instance;
		}
	}

	virtual std::string decompile(int level) const override {
		switch(member_id) {
			case publica_e: return "public";
			case privatea_e: return "private";
			case statica_e: return "static";
			case consta_e: return "const";
		}
		assert(false);
		return "";
	}
};

#include "../eh_libclasses.hpp"

EH_INITIALIZER(Attribute);

#endif /* EH_ATTRIBUTE_H_ */
