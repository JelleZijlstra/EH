/*
 * Attribute
 * Enum class used for method attribute (public, private, const, static)
 */
#include "std_lib_includes.hpp"
#include "Attribute.hpp"

std::map<Attribute::attribute_enum, ehval_p> Attribute::instances;

#define REGISTER_MEMBER(name, enum) obj->add_enum_member(#name "Attribute", {}, parent, static_cast<int>(Attribute::enum))

EH_INITIALIZER(Attribute) {
	REGISTER_MEMBER(public, publica_e);
	REGISTER_MEMBER(private, privatea_e);
	REGISTER_MEMBER(static, statica_e);
	REGISTER_MEMBER(const, consta_e);
}
