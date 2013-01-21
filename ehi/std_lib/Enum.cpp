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
	REGISTER_METHOD(Enum, new);
	REGISTER_METHOD(Enum, toString);
	REGISTER_METHOD_RENAME(Enum, operator_colon, "operator()");
	REGISTER_METHOD(Enum, compare);
	REGISTER_METHOD(Enum, typeId);
	REGISTER_METHOD(Enum, type);
	REGISTER_METHOD(Enum, isConstructor);
	REGISTER_METHOD_RENAME(Enum, operator_arrow, "operator->");
	REGISTER_METHOD(Enum, map);

	// register a type id for Enum_Instance
	parent->repo.register_inbuilt_class<Enum_Instance>(Null::make());
}

ehval_p Enum::make_enum_class(const char *name, ehval_p scope, EHInterpreter *parent) {
	// create wrapper object
	ehobj_t *enum_obj = new ehobj_t();
	ehval_p ret = Object::make(enum_obj, parent);

	// create enum object
	ehval_p enum_class_obj = Enum::make(name);

	// register class
	const unsigned int type_id = parent->repo.register_class(name, ret);

	// inherit from Enum
	enum_obj->inherit(parent->repo.get_primitive_class<Enum>());

	enum_obj->type_id = type_id;
	enum_obj->parent = scope;
	enum_obj->object_data = enum_class_obj;
	return ret;
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
	join(out, member_map, ", ");
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
 * @description Converts an enum to string. Returns a representation similar to
 * the code used to create an enum, but omitting any methods.
 * @argument None
 * @returns String representation of the enum.
 */
EH_METHOD(Enum, toString) {
	if(obj->is_a<Object>()) {
		// then it must be an enum class object
		ASSERT_RESOURCE(Enum, "Enum.toString");
		return String::make(strdup(data->to_string().c_str()));
	} else if(obj->is_a<Enum_Instance>()) {
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
	} else {
		throw_TypeError("Invalid base object for Enum.toString", obj, ehi);
		return nullptr;
	}
}

/*
 * @description Instantiate an enum member. This throws an error unless the
 * object it is called on is a good constructor.
 * @argument Constructor arguments
 * @returns New enum instance
 */
EH_METHOD(Enum, operator_colon) {
	ASSERT_OBJ_TYPE(Enum_Instance, "Enum.operator()");
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
EH_METHOD(Enum, compare) {
	if(obj->is_a<Object>()) {
		ASSERT_OBJ_TYPE(Enum, "Enum.compare");
		args->assert_type<Object>("Enum.compare", ehi);
		return Integer::make(intcmp(_obj->get<Object>()->type_id, args->get<Object>()->type_id));
	} else {
		ASSERT_TYPE(obj, Enum_Instance, "Enum.compare");
		ASSERT_TYPE(args, Enum_Instance, "Enum.compare");
		auto data = obj->get<Enum_Instance>();
		return Integer::make(data->compare(args->get<Enum_Instance>(), ehi, obj));
	}
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
 * @description Return the type id of the current enum.
 * @argument None
 * @returns Integer
 */
EH_METHOD(Enum, typeId) {
	if(obj->is_a<Object>()) {
		ASSERT_OBJ_TYPE(Enum, "Enum.typeId");
		return Integer::make(static_cast<int>(_obj->get<Object>()->type_id));
	} else {
		ASSERT_RESOURCE(Enum_Instance, "Enum.typeId");
		return Integer::make(static_cast<int>(data->type_id));
	}
}

/*
 * @description Return the name of the current enum class.
 * @argument None
 * @returns String
 */
EH_METHOD(Enum, type) {
	if(obj->is_a<Object>()) {
		ASSERT_RESOURCE(Enum, "Enum.type");
		return String::make(strdup(data->name.c_str()));
	} else {
		ASSERT_RESOURCE(Enum_Instance, "Enum.type");
		auto parent_enum = data->get_parent_enum(ehi);
		auto name = parent_enum->get<Enum>()->name.c_str();
		return String::make(strdup(name));
	}
}

/*
 * @description Checks whether this enum is an acceptable constructor.
 * @argument None
 * @returns Bool
 */
EH_METHOD(Enum, isConstructor) {
	ASSERT_RESOURCE(Enum_Instance, "Enum.isConstructor");
	return Bool::make(data->is_constructor());
}

/*
 * @description Returns the nth constructor argument of an enum instance.
 * @argument Integer
 * @returns Argument
 */
EH_METHOD(Enum, operator_arrow) {
	ASSERT_RESOURCE(Enum_Instance, "Enum.operator->");
	if(data->is_constructor()) {
		throw_MiscellaneousError("Cannot call Enum.operator-> on constructor", ehi);
	}
	ASSERT_TYPE(args, Integer, "Enum.operator->");
	const int index = args->get<Integer>();
	if(index < 0 || index >= static_cast<int>(data->nmembers)) {
		throw_ArgumentError_out_of_range("Enum.operator->", args, ehi);
	}
	return data->get(static_cast<unsigned int>(index));
}

/*
 * @description Applies the specified function to all arguments of the
 * enum instance and returns a new instance of the same member.
 */
EH_METHOD(Enum, map) {
	ASSERT_RESOURCE(Enum_Instance, "Enum.map");
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
