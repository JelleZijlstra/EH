/*
 * Enum
 * Implements enums. See <a href="/syntax">Syntax</a> for more information on
 * enums.
 */

#include "Enum.hpp"

#include <sstream>

EH_INITIALIZER(Enum) {
	REGISTER_METHOD(Enum, new);
	REGISTER_METHOD(Enum, size);
	REGISTER_METHOD(Enum, toString);
	REGISTER_CLASS(Enum, Member);
	REGISTER_CLASS(Enum, Instance);
	parent->enum_id = obj->type_id;
}

ehval_p Enum::t::make(const char *name, EHI *ehi) {
	// prototype object
	ehobj_t *contents_obj = new ehobj_t;
	ehval_p contents = Object::make(contents_obj, ehi->get_parent());

	int enum_id = ehi->get_parent()->repo.get_primitive_id<Enum>();
	ehval_p data = Enum::make(new Enum::t(name, contents), ehi->get_parent());
	ehval_p ret = ehi->get_parent()->resource_instantiate(enum_id, data);

	// connect the prototype object
	contents_obj->parent = ret;
	contents_obj->inherit(ehi->get_parent()->base_object);
	ehmember_p member = ehmember_t::make(attributes_t::make_const(), contents);
	ret->get<Object>()->insert("prototype", member);
	return ret;
}

Enum::t *Enum::t::extract_enum(ehval_p obj) {
	return obj->data()->get<Enum>();
}

void Enum::t::add_member(ehval_p e, const char *name, ehval_p member, EHI *ehi) {
	ehmember_p class_member;
	class_member->attribute = attributes_t::make_const();
	class_member->value = member;

	e->get<Object>()->insert(name, class_member);

	Enum::t *e_obj = Enum::t::extract_enum(e);
	e_obj->nmembers++;
	e_obj->members.push_back(member);
}

void Enum::t::add_nullary_member(ehval_p e, const char *name, EHI *ehi) {
	ehval_p member = Enum_Member::t::make(e, name, ehi);
	add_member(e, name, member, ehi);
}

void Enum::t::add_member_with_arguments(ehval_p e, const char *name, Enum_Member::t::params_t params, EHI *ehi) {
	ehval_p member = Enum_Member::t::make(e, name, params, ehi);
	add_member(e, name, member, ehi);
}

std::string Enum::t::toString() const {
	std::ostringstream out;
	out << "enum " << name << "\n\t";
	for(unsigned int i = 0; i < nmembers; i++) {
		Enum_Member::t *member = members[i]->data()->get<Enum_Member>();
		out << member->toString();
		if(i < nmembers - 1) {
			out << ", ";
		}
	}
	out << "\nend";
	return out.str();
}

/*
 * @description Directly creating a new Enum is not allowed; this method always throws an error.
 * @argument Irrelevant
 * @returns N/A
 */
EH_METHOD(Enum, new) {
	throw_TypeError("Cannot instantiate Enum", ehi->get_parent()->repo.get_primitive_class<Enum>(), ehi);
	return nullptr;
}

/*
 * @description Returns the number of elements in an enum.
 * @argument None
 * @returns Integer
 */
EH_METHOD(Enum, size) {
	ASSERT_RESOURCE(Enum, "Enum.size");
	return Integer::make(data->size());
}

/*
 * @description Converts an enum to string. Returns a representation similar to
 * the code used to create an enum, but omitting any methods.
 * @argument None
 * @returns String representation of the enum.
 */
EH_METHOD(Enum, toString) {
	ASSERT_RESOURCE(Enum, "Enum.toString");
	return String::make(strdup(data->toString().c_str()));
}

EH_INITIALIZER(Enum_Member) {
	REGISTER_METHOD(Enum_Member, new);
	REGISTER_METHOD(Enum_Member, toString);
	REGISTER_METHOD_RENAME(Enum_Member, operator_colon, "operator()");
	parent->enum_member_id = obj->type_id;
}

ehval_p Enum_Member::t::make(ehval_p e, Enum_Member::t *em, EHI *ehi) {
	int enum_member_id = ehi->get_parent()->repo.get_primitive_id<Enum_Member>();
	ehval_p data = Enum_Member::make(em, ehi->get_parent());
	ehval_p ret = ehi->get_parent()->resource_instantiate(enum_member_id, data);
	ret->get<Object>()->inherit(Enum::t::extract_enum(e)->contents);
	return ret;
}

ehval_p Enum_Member::t::make(ehval_p e, const char *name, EHI *ehi) {
	return Enum_Member::t::make(e, new Enum_Member::t(e, name), ehi);
}

ehval_p Enum_Member::t::make(ehval_p e, const char *name, params_t &params, EHI *ehi) {
	return Enum_Member::t::make(e, new Enum_Member::t(e, name, params.size(), params), ehi);
}

std::string Enum_Member::t::toString() const {
	if(size == 0) {
		return name;
	} else {
		std::ostringstream out;
		out << name << "(";
		for(int i = 0; i < size; i++) {
			out << params[i];
			if(i < size - 1) {
				out << ", ";
			}
		}
		out << ")";
		return out.str();
	}
}

EH_METHOD(Enum_Member, new) {
	throw_TypeError("Cannot instantiate Enum.Member", ehi->get_parent()->repo.get_primitive_class<Enum_Member>(), ehi);
	return nullptr;
}

EH_METHOD(Enum_Member, operator_colon) {
	ASSERT_RESOURCE(Enum_Member, "Enum.Member.operator()");
	const int size = data->size;
	if(size == 0) {
		throw_TypeError("Cannot instantiate nullary Enum.Member", ehi->get_parent()->repo.get_primitive_class<Enum_Member>(), ehi);
		return nullptr;
	} else if(size == 1) {
		Enum_Instance::t::args_t params(1);
		params[0] = args;
		return Enum_Instance::t::make(obj, params, ehi);
	} else {
		ASSERT_NARGS(size, "Enum.Member.operator()");

		Enum_Instance::t::args_t params(size);
		Tuple::t *tuple = args->get<Tuple>();
		for(int i = 0; i < size; i++) {
			params[i] = tuple->get(i);
		}
		return Enum_Instance::t::make(obj, params, ehi);
	}
}

EH_METHOD(Enum_Member, toString) {
	ASSERT_RESOURCE(Enum_Member, "Enum.Member.toString");
	return String::make(strdup(data->toString().c_str()));
}

EH_INITIALIZER(Enum_Instance) {
	REGISTER_METHOD(Enum_Instance, toString);
	REGISTER_METHOD(Enum_Instance, compare);
	parent->enum_instance_id = obj->type_id;
}

EH_METHOD(Enum_Instance, toString) {
	ASSERT_RESOURCE(Enum_Instance, "Enum.Instance.toString");
	return String::make(strdup(data->toString(ehi, obj).c_str()));
}

EH_METHOD(Enum_Instance, compare) {
	ASSERT_RESOURCE(Enum_Instance, "Enum.Instance.compare");
	args->assert_type<Object>("Enum.Instance.compare", ehi);
	ehval_p obj_data = args->get<Object>()->object_data;
	ASSERT_TYPE(obj_data, Enum_Instance, "Enum.Instance.compare");
	Enum_Instance::t *rhs = obj_data->get<Enum_Instance>();
	return Integer::make(data->compare(rhs, ehi, obj));
}

int Enum_Instance::t::compare(Enum_Instance::t *rhs, EHI *ehi, ehcontext_t context) {
	const int member_compare = member_ptr->naive_compare(rhs->member_ptr);
	if(member_compare != 0) {
		return member_compare;
	}
	// so they are at least instances of the same Enum.Member
	Enum_Member::t *em = member_ptr->get<Enum_Member>();
	for(int i = 0, size = em->size; i < size; i++) {
		int arg_compare = ehi->compare(args[i], rhs->args[i], context);
		if(arg_compare != 0) {
			return arg_compare;
		}
	}
	// now they're equal
	return 0;
}

std::string Enum_Instance::t::toString(EHI *ehi, ehcontext_t context) {
	Enum_Member::t *em = member_ptr->get<Enum_Member>();
	std::ostringstream out;
	out << em->name << "(";
	const int size = em->size;
	for(int i = 0; i < size; i++) {
		ehval_p stringified = ehi->toString(args[i], context);
		out << stringified->get<String>();
		if(i < size - 1) {
			out << ", ";
		}
	}
	out << ")";
	return out.str();
}

ehval_p Enum_Instance::t::make(ehval_p member, args_t args, EHI *ehi) {
	int enum_instance_id = ehi->get_parent()->enum_instance_id;
	ehval_p ei = Enum_Instance::make(new Enum_Instance::t(member, args), ehi->get_parent());
	ehval_p ret = ehi->get_parent()->resource_instantiate(enum_instance_id, ei);
	ehobj_t *obj = ret->get<Object>();
	Enum_Member::t *em = member->get<Enum_Member>();
	for(int i = 0, len = em->size; i < len; i++) {
		ehmember_p member = ehmember_t::make(attributes_t::make_const(), args[i]);
		obj->insert(em->params[i], member);
	}
	obj->inherit(Enum::t::extract_enum(em->parent_enum)->contents);
	return ret;
}
