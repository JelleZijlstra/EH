#include "Enum.h"

#include <sstream>

EH_INITIALIZER(Enum) {
	REGISTER_METHOD(Enum, new);
	REGISTER_METHOD(Enum, size);
	REGISTER_METHOD(Enum, toString);
	REGISTER_CLASS(Enum, Member);
	REGISTER_CLASS(Enum, Instance);
	parent->enum_id = obj->type_id;
}

ehretval_p Enum::make(const char *name, EHI *ehi) {
	int enum_id = ehi->get_parent()->enum_id;
	LibraryBaseClass *obj = static_cast<LibraryBaseClass *>(new Enum(name));
	return ehi->get_parent()->resource_instantiate(enum_id, obj);
}

Enum *Enum::extract_enum(ehretval_p obj) {
	return static_cast<Enum *>(obj->get_objectval()->object_data->get_resourceval());
}

void Enum::add_member(ehretval_p e, const char *name, ehretval_p member, EHI *ehi) {
	ehmember_p class_member;
	class_member->attribute = attributes_t::make_const();
	class_member->value = member;

	e->get_objectval()->insert(name, class_member);

	Enum *e_obj = Enum::extract_enum(e);
	e_obj->nmembers++;
	e_obj->members.push_back(member);	
}

void Enum::add_nullary_member(ehretval_p e, const char *name, EHI *ehi) {
	ehretval_p member = Enum_Member::make(e, name, ehi);
	add_member(e, name, member, ehi);
}

void Enum::add_member_with_arguments(ehretval_p e, const char *name, Enum_Member::params_t params, EHI *ehi) {
	ehretval_p member = Enum_Member::make(e, name, params, ehi);
	add_member(e, name, member, ehi);
}

std::string Enum::to_string() const {
	std::ostringstream out;
	out << "enum " << name << "\n\t";
	for(unsigned int i = 0; i < nmembers; i++) {
		Enum_Member *member = static_cast<Enum_Member *>(members[i]->get_objectval()->object_data->get_resourceval());
		out << member->to_string();
		if(i < nmembers - 1) {
			out << ", ";
		}
	}
	out << "\nend";
	return out.str();	
}

EH_METHOD(Enum, new) {
	throw_TypeError("Cannot instantiate Enum", ehi->get_parent()->enum_id, ehi);
	return NULL;
}

EH_METHOD(Enum, size) {
	ASSERT_RESOURCE(Enum, "Enum.size");
	return ehretval_t::make_int(data->size());
}

EH_METHOD(Enum, toString) {
	ASSERT_RESOURCE(Enum, "Enum.toString");
	return ehretval_t::make_string(strdup(data->to_string().c_str()));
}

EH_INITIALIZER(Enum_Member) {
	REGISTER_METHOD(Enum_Member, new);
	REGISTER_METHOD(Enum_Member, toString);
	REGISTER_METHOD_RENAME(Enum_Member, operator_colon, "operator()");
	parent->enum_member_id = obj->type_id;
}

ehretval_p Enum_Member::make(ehretval_p e, const char *name, EHI *ehi) {
	int enum_member_id = ehi->get_parent()->enum_member_id;
	LibraryBaseClass *obj = static_cast<LibraryBaseClass *>(new Enum_Member(e, name));
	return ehi->get_parent()->resource_instantiate(enum_member_id, obj);
}

ehretval_p Enum_Member::make(ehretval_p e, const char *name, params_t &params, EHI *ehi) {
	int enum_member_id = ehi->get_parent()->enum_member_id;
	LibraryBaseClass *obj = static_cast<LibraryBaseClass *>(new Enum_Member(e, name, params.size(), params));
	return ehi->get_parent()->resource_instantiate(enum_member_id, obj);	
}

std::string Enum_Member::to_string() const {
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
	throw_TypeError("Cannot instantiate Enum.Member", ehi->get_parent()->enum_member_id, ehi);
	return NULL;
}

EH_METHOD(Enum_Member, operator_colon) {
	ASSERT_RESOURCE(Enum_Member, "Enum.Member.operator()");
	const int size = data->size;
	if(size == 0) {
		throw_TypeError("Cannot instantiate nullary Enum.Member", ehi->get_parent()->enum_member_id, ehi);
		return NULL;
	} else if(size == 1) {
		Enum_Instance::args_t params(1);
		params[0] = args;
		return Enum_Instance::make(obj, params, ehi);
	} else {
		ASSERT_NARGS(size, "Enum.Member.operator()");

		Enum_Instance::args_t params(size);
		ehtuple_t *tuple = args->get_tupleval();
		for(int i = 0; i < size; i++) {
			params[i] = tuple->get(i);
		}
		return Enum_Instance::make(obj, params, ehi);
	}
}

EH_METHOD(Enum_Member, toString) {
	ASSERT_RESOURCE(Enum_Member, "Enum.Member.toString");
	return ehretval_t::make_string(strdup(data->to_string().c_str()));
}

EH_INITIALIZER(Enum_Instance) {
	REGISTER_METHOD(Enum_Instance, toString);
	REGISTER_METHOD(Enum_Instance, compare);
	parent->enum_instance_id = obj->type_id;
}

EH_METHOD(Enum_Instance, toString) {
	ASSERT_RESOURCE(Enum_Instance, "Enum.Instance.toString");
	return ehretval_t::make_string(strdup(data->to_string(ehi, obj).c_str()));
}

EH_METHOD(Enum_Instance, compare) {
	ASSERT_RESOURCE(Enum_Instance, "Enum.Instance.compare");
	ASSERT_TYPE(args, object_e, "Enum.Instance.compare");
	ehretval_p obj_data = args->get_objectval()->object_data;
	ASSERT_TYPE(obj_data, resource_e, "Enum.Instance.compare");
	Enum_Instance *rhs = static_cast<Enum_Instance *>(obj_data->get_resourceval());
	return ehretval_t::make_int(data->compare(rhs, ehi, obj));
}

int Enum_Instance::compare(Enum_Instance *rhs, EHI *ehi, ehcontext_t context) {
	const int member_compare = member_ptr->naive_compare(rhs->member_ptr);
	if(member_compare != 0) {
		return member_compare;
	}
	// so they are at least instances of the same Enum.Member
	Enum_Member *em = static_cast<Enum_Member *>(member_ptr->get_resourceval());
	for(int i = 0, size = em->size; i < size; i++) {
		int arg_compare = ehi->compare(args[i], rhs->args[i], context);
		if(arg_compare != 0) {
			return arg_compare;
		}
	}
	// now they're equal
	return 0;
}

std::string Enum_Instance::to_string(EHI *ehi, ehcontext_t context) {
	Enum_Member *em = static_cast<Enum_Member *>(member_ptr->get_resourceval());
	std::ostringstream out;
	out << em->name << "(";
	const int size = em->size;
	for(int i = 0; i < size; i++) {
		ehretval_p stringified = ehi->to_string(args[i], context);
		out << stringified->get_stringval();
		if(i < size - 1) {
			out << ", ";
		}
	}
	out << ")";
	return out.str();
}

ehretval_p Enum_Instance::make(ehretval_p member, args_t args, EHI *ehi) {
	int enum_instance_id = ehi->get_parent()->enum_instance_id;
	Enum_Instance *ei = new Enum_Instance(member, args);
	ehretval_p ret = ehi->get_parent()->resource_instantiate(enum_instance_id, static_cast<LibraryBaseClass *>(ei));
	ehobj_t *obj = ret->get_objectval();
	Enum_Member *em = static_cast<Enum_Member *>(member->get_resourceval());
	for(int i = 0, len = em->size; i < len; i++) {
		ehmember_p member;
		member->attribute = attributes_t::make_const();
		member->value = args[i];
		obj->insert(em->params[i], member);
	}
	return ret;
}
