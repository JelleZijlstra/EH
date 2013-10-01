/*
 * Enum
 * Implements enums. See <a href="/syntax">Syntax</a> for more information on
 * enums.
 */
#include "std_lib_includes.hpp"

#include "Enum.hpp"

#include "ArgumentError.hpp"
#include "MiscellaneousError.hpp"
#include "Null.hpp"
#include "Node.hpp"
#include "Attribute.hpp"

#include <sstream>
#include <memory>

// Not nearly as generic as it should be - for that we'd probably need lambdas
template<class S, class T>
void join(S &stream, const T &container, const std::string separator) {
	auto begin = container.begin();
	auto end = container.end();

	stream << begin->second.to_string();
	++begin;

	while(begin != end) {
		stream << separator;
		stream << begin->second.to_string();

		++begin;
	}
}

EH_INITIALIZER(Enum) {
	REGISTER_METHOD(Enum, toString);
	REGISTER_METHOD_RENAME(Enum, operator_colon, "operator()");
	REGISTER_METHOD_RENAME(Enum, operator_colon, "new");
	REGISTER_CLASS(Enum, Instance);
	INHERIT_LIBRARY(Class);
}

EH_INITIALIZER(Enum_Instance) {
	REGISTER_METHOD(Enum_Instance, isConstructor);
	REGISTER_METHOD_RENAME(Enum_Instance, operator_colon, "operator()");
	REGISTER_METHOD_RENAME(Enum_Instance, operator_arrow, "operator->");
	REGISTER_METHOD(Enum_Instance, map);
	REGISTER_METHOD(Enum_Instance, compare);
	REGISTER_METHOD(Enum_Instance, toString);
}

void Enum::t::add_enum_member(const char *name, const std::vector<std::string> &params, EHInterpreter *interpreter_parent, unsigned int member_id) {
	member_info member(name, params);

	if(member_id == 0) {
		member_id = n_enum_members + 1;
	}
	enum_members[member_id] = member;
	n_enum_members++;

    // insert member into the class
    auto ei = new Enum_Instance::t(type_id, member_id, static_cast<unsigned int>(params.size()), nullptr);
    auto ei_obj = Enum_Instance::make(ei, interpreter_parent);
    ehmember_p enum_member = ehmember_t::make(attributes_t::make_const(), ei_obj);
    insert(name, enum_member);
}

ehval_p Enum::make_enum_class(const char *name, ehval_p scope, EHInterpreter *parent) {
	// create enum object
	ehval_p enum_obj = Enum::make(name);

	// register class
	const unsigned int type_id = parent->repo.register_class(name, enum_obj);

	Enum::t *inner_obj = enum_obj->get<Enum>();
	inner_obj->type_id = type_id;
	inner_obj->parent = scope;

	// so that members can access Enum_Instance methods
	inner_obj->inherit(parent->repo.get_primitive_class<Enum_Instance>());
	return enum_obj;
}

unsigned int register_enum_class(ehobj_t *obj, const enum_initializer init_func, const char *name, const attributes_t attributes, EHInterpreter *interpreter_parent) {
    const ehval_p ret = Enum::make_enum_class(name, interpreter_parent->global_object, interpreter_parent);
    auto enum_obj = ret->get<Enum>();

    init_func(enum_obj, interpreter_parent);

    ehmember_p member = ehmember_t::make(attributes, ret);
    obj->insert(name, member);

    return enum_obj->type_id;
}

const std::string Enum::member_info::to_string() const {
	std::ostringstream out;
	out << name;
	const size_t size = members.size();
	if(size > 0) {
		out << "(";
		for(unsigned int i = 0; i < size; i++) {
			out << members[i];
			if(i < size - 1) {
				out << ", ";
			}
		}
		out << ")";
	}
	return out.str();
}

const std::string Enum::t::to_string() const {
	std::ostringstream out;
	out << "enum " << name << "\n\t";
	join(out, enum_members, ", ");
	out << "\nend";
	return out.str();
}

void Enum_Instance::printvar(printvar_set &set, int level, EHI *ehi) {
	void *ptr = static_cast<void *>(value);
	if(set.count(ptr) > 0) {
		std::cout << "(recursion)" << std::endl;
	} else {
		set.insert(ptr);
		auto &parent_enum = value->get_parent_enum(ehi);
		auto &info = value->get_member_info(ehi);
		if((value->nmembers > 0) && value->members == nullptr) {
			std::cout << "@enum constructor " << info.name << " <" << parent_enum->get<Enum>()->name << ">: ";
			for(unsigned int i = 0; i < value->nmembers; i++) {
				std::cout << info.members[i];
				if(i < value->nmembers - 1) {
					std::cout << ", ";
				}
			}
		} else {
			std::cout << "@enum instance " << info.name << " <" << parent_enum->get<Enum>()->name << "> [\n";
			for(unsigned int i = 0; i < value->nmembers; i++) {
				add_tabs(std::cout, level + 1);
				std::cout << info.members[i] << ": ";
				value->members[i]->printvar(set, level + 1, ehi);
			}
			add_tabs(std::cout, level);
			std::cout << "]";
		}
		std::cout << std::endl;
	}
}

const std::string Enum_Instance::t::decompile(int level) const {
	// hack to allow decompiling Node and Attribute easily
	switch(type_id) {
		case Node::node_type_id:
			return (static_cast<const Node *>(this))->decompile(level);
		case Attribute::attribute_type_id:
			return (static_cast<const Attribute *>(this))->decompile(level);
		default:
			return "";
	}
}

/*
 * @description Directly creating a new Enum is not allowed; this method always throws an error.
 * @argument Irrelevant
 * @returns N/A
 */
EH_METHOD(Enum, operator_colon) {
	throw_TypeError("Cannot instantiate Enum", ehi->get_parent()->repo.get_primitive_class<Enum>(), ehi);
}

/*
 * @description Converts an enum to string. Returns a representation similar to
 * the code used to create an enum, but omitting any methods.
 * @argument None
 * @returns String representation of the enum.
 */
EH_METHOD(Enum, toString) {
	// then it must be an enum class object
	obj->assert_type<Enum>("Enum.toString", ehi);
	return String::make(strdup(obj->get<Enum>()->to_string().c_str()));
}

EH_METHOD(Enum_Instance, toString) {
	obj->assert_type<Enum_Instance>("Enum.Instance.toString", ehi);
	auto data = obj->get<Enum_Instance>();
	auto &info = data->get_member_info(ehi);
	if(data->members == nullptr) {
		return String::make(strdup(info.to_string().c_str()));
	} else {
		std::ostringstream out;
		out << info.name << "(";
		for(unsigned int i = 0; i < data->nmembers; i++) {
			out << ehi->toString(data->members[i], obj)->get<String>();
			if(i < data->nmembers - 1) {
				out << ", ";
			}
		}
		out << ")";
		return String::make(strdup(out.str().c_str()));
	}
}

/*
 * @description Instantiate an enum member. This throws an error unless the
 * object it is called on is a good constructor.
 * @argument Constructor arguments
 * @returns New enum instance
 */
EH_METHOD(Enum_Instance, operator_colon) {
	ASSERT_OBJ_TYPE(Enum_Instance, "Enum.Instance.operator()");
	auto data = obj->get<Enum_Instance>();
	const unsigned int size = data->nmembers;
	if(size == 0) {
		throw_MiscellaneousError("Cannot instantiate nullary Enum member", ehi);
		return nullptr;
	} else if(data->members != nullptr) {
		throw_MiscellaneousError("Cannot instantiate existing Enum member", ehi);
		return nullptr;
	} else {
		ehval_p *params;
		if(size > 1) {
			ASSERT_NARGS(size, "Enum.operator()");
			params = new ehval_p[size]();
			auto tuple = args->get<Tuple>();
			for(unsigned int i = 0; i < size; i++) {
				params[i] = tuple->get(i);
			}
		} else {
			params = new ehval_p[1]();
			params[0] = args;
		}

		auto val = new Enum_Instance::t(data->type_id, data->member_id, size, params);
		return Enum_Instance::make(val, ehi->get_parent());
	}
}

/*
 * @description Compares one enum to another. Checks, in order, whether the
 * arguments are either both enum classes or enum mebers; if they are members,
 * whether the type ids match; if so, whether the member ids match; if so,
 * whether both are instantiated; if so, whether the arguments match.
 * @argument Object to compare to
 * @returns Integer
 */
EH_METHOD(Enum_Instance, compare) {
	obj->assert_type<Enum_Instance>("Enum.Instance.compare", ehi);
	args->assert_type<Enum_Instance>("Enum.Instance.compare", ehi);
	auto data = obj->get<Enum_Instance>();
	return Integer::make(data->compare(args->get<Enum_Instance>(), ehi, obj));
}

int Enum_Instance::t::type_compare(Enum_Instance::t *rhs) {
	if(type_id != rhs->type_id) {
		return intcmp(type_id, rhs->type_id);
	} else if(member_id != rhs->member_id) {
		return intcmp(member_id, rhs->member_id);
	} else {
		return 0;
	}
}

int Enum_Instance::t::compare(Enum_Instance::t *rhs, EHI *ehi, ehcontext_t context) {
	const int type_comparison = type_compare(rhs);
	if(type_comparison != 0) {
		return type_comparison;
	} else if(members == nullptr) {
		return (rhs->members == nullptr) ? 0 : -1;
	} else if(rhs->members == nullptr) {
		return 1;
	} else {
		// so they are at least instances of the same Enum.Member
		for(unsigned int i = 0, size = nmembers; i < size; i++) {
			const int arg_compare = ehi->compare(members[i], rhs->members[i], context);
			if(arg_compare != 0) {
				return arg_compare;
			}
		}
		// now they're equal
		return 0;
	}
}

/*
 * @description Checks whether this enum is an acceptable constructor.
 * @argument None
 * @returns Bool
 */
EH_METHOD(Enum_Instance, isConstructor) {
	obj->assert_type<Enum_Instance>("Enum.Instance.isConstructor", ehi);
	return Bool::make(obj->get<Enum_Instance>()->is_constructor());
}

/*
 * @description Returns the nth constructor argument of an enum instance.
 * @argument Integer
 * @returns Argument
 */
EH_METHOD(Enum_Instance, operator_arrow) {
	obj->assert_type<Enum_Instance>("Enum.Instance.operator->", ehi);
	auto data = obj->get<Enum_Instance>();
	if(data->is_constructor()) {
		throw_MiscellaneousError("Cannot call Enum.operator-> on constructor", ehi);
	}
	args->assert_type<Integer>("Enum.Instance.operator->", ehi);
	const int index = args->get<Integer>();
	if(index < 0 || index >= static_cast<int>(data->nmembers)) {
		throw_ArgumentError_out_of_range("Enum.Instance.operator->", args, ehi);
	}
	return data->get(static_cast<unsigned int>(index));
}

/*
 * @description Applies the specified function to all arguments of the
 * enum instance and returns a new instance of the same member.
 */
EH_METHOD(Enum_Instance, map) {
	obj->assert_type<Enum_Instance>("Enum.Instance.map", ehi);
	auto data = obj->get<Enum_Instance>();
	if(data->is_constructor()) {
		throw_MiscellaneousError("Cannot call Enum.map on constructor", ehi);
	}
	const unsigned int size = data->nmembers;
	// mapping over nullary member doesn't do anything
	if(size == 0) {
		return obj;
	}
	std::unique_ptr<ehval_p[]> params(new ehval_p[size]());
	for(unsigned int i = 0; i < size; i++) {
		params[i] = ehi->call_function(args, data->get(i), obj);
	}
	// convert to ehval_p only now
	// if we do it before the call_function, GC may hit while this function is running and kill our ehval_ps
	ehval_p *weak_params = new ehval_p[size]();
	for(unsigned int i = 0; i < size; i++) {
		weak_params[i] = params[i];
	}
	auto val = new Enum_Instance::t(data->type_id, data->member_id, size, weak_params);
	return Enum_Instance::make(val, ehi->get_parent());
}
