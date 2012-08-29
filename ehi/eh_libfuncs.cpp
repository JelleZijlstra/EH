/*
 * eh_libfuncs.c
 * Jelle Zijlstra, December 2011
 *
 * Contains definitions of EH library functions
 */
#include "eh.h"
#include "eh_libfuncs.h"
#include "eh.bison.hpp"
#include <cmath>

// TODO: build an argument getter that sets arguments to local variables, checks
// argument number, etcetera

// Note that library functions get their arguments in reverse order

/*
 * printvar
 */

class printvar_t {
private:
	std::map<const void *, bool> seen;
	EHI *ehi;
	
	void retval(ehretval_p in);
	void array(eharray_t *in);
	void object(ehobj_t *in);
	void tuple(ehtuple_t *in);

	printvar_t(const printvar_t&) : seen(), ehi() {
		throw "Not allowed";
	}
	printvar_t operator=(const printvar_t&) {
		throw "Not allowed";
	}
public:
	printvar_t(ehretval_p in, EHI *_ehi) : seen(), ehi(_ehi) {
		this->retval(in);
	}
};

// helper functions for printvar
void printvar_t::retval(ehretval_p in) {
	switch(in->type()) {
		case null_e:
			printf("null\n");
			break;
		case int_e:
			printf("@int %d\n", in->get_intval());
			break;
		case string_e:
			printf("@string '%s'\n", in->get_stringval());
			break;
		case bool_e:
			if(in->get_boolval())
				printf("@bool true\n");
			else
				printf("@bool false\n");
			break;
		case array_e:
			if(this->seen.count((void *)in->get_arrayval()) == 0) {
				this->seen[(void *)in->get_arrayval()] = true;
				printf("@array [\n");
				this->array(in->get_arrayval());
				printf("]\n");
			} else {
				printf("(recursion)\n");
			}
			break;
		case tuple_e:
		  if(this->seen.count((void *)in->get_tupleval()) == 0) {
		    this->seen[(void *)in->get_tupleval()] = true;
		    printf("@tuple <%d> [\n", in->get_tupleval()->size());
		    this->tuple(in->get_tupleval());
		    printf("]\n");
		  } else {
		    printf("(recursion)");
		  }
		  break;
		case object_e: {
			ehobj_t *obj = in->get_objectval();
			if(obj->type_id != func_e || obj->object_data->type() != func_e) {
				if(this->seen.count((void *)obj) == 0) {
					this->seen[(void *)obj] = true;
					//TODO: print the classname somehow
					const char *name = ehi->repo.get_name(obj->type_id).c_str();
					printf("@object <%s> [\n", name);
					this->object(obj);
					printf("]\n");
				} else {
					printf("(recursion)\n");
				}
			} else {
				ehfunc_t *f = obj->object_data->get_funcval();
				printf("@function <");
				switch(f->type) {
					case user_e:
						printf("user");
						break;
					case lib_e:
						printf("library");
						break;
				}
				printf(">: ");
				for(int i = 0; i < f->argcount; i++) {
					printf("%s", f->args[i].name.c_str());
					if(i + 1 < f->argcount) {
						printf(", ");
					}
				}
				printf("\n");
			}
			break;
		}
		case func_e: {
			ehfunc_t *f = in->get_funcval();
			printf("@function <");
			switch(f->type) {
				case user_e:
					printf("user");
					break;
				case lib_e:
					printf("library");
					break;
			}
			printf(">: ");
			for(int i = 0; i < f->argcount; i++) {
				printf("%s", f->args[i].name.c_str());
				if(i + 1 < f->argcount) {
					printf(", ");
				}
			}
			printf("\n");
			break;
		}
		case type_e:
			printf("@type %s\n", get_typestring(in->get_typeval()));
			break;
		case op_e:
			printf("@op %d\n", in->get_opval()->op);
			break;
		case attribute_e:
			printf("@attribute %d\n", in->get_attributeval());
			break;
		case attributestr_e:
			printf("@attributestr\n");
			break;
		case range_e: {
			ehrange_t *range = in->get_rangeval();
			if(this->seen.count((void *)range) == 0) {
				printf("@range [\n");
				this->retval(range->min);
				this->retval(range->max);
				printf("]\n");
				this->seen[(void *)range] = true;
			} else {
				printf("(recursion)\n");
			}
			break;
		}
		case float_e:
			printf("@float %f\n", in->get_floatval());
			break;
		case resource_e:
			printf("@resource\n");
			break;
		case binding_e:
			// pretend it's just a method
			this->retval(in->get_bindingval()->method);
			break;
		case hash_e:
			printf("@hash [\n");
			HASH_FOR_EACH(in->get_hashval(), i) {
				printf("'%s': ", i->first.c_str());
				this->retval(i->second);
			}
			printf("]\n");
		case super_class_e:
			printf("@parent class\n");
			break;
	}
	return;
}
void printvar_t::object(ehobj_t *in) {
	OBJECT_FOR_EACH(in, curr) {
		// ignore $this
		if(curr->first.compare("this") == 0) {
			continue;
		}
		
		printf("%s <", curr->first.c_str());
		switch(curr->second->attribute.visibility) {
			case public_e:
				printf("public,");
				break;
			case private_e:
				printf("private,");
				break;
		}
		switch(curr->second->attribute.isstatic) {
			case static_e:
				printf("static,");
				break;
			case nonstatic_e:
				printf("non-static,");
				break;
		}
		switch(curr->second->attribute.isconst) {
			case const_e:
				printf("constant");
				break;
			case nonconst_e:
				printf("non-constant");
				break;
		}
		printf(">: ");
		this->retval(curr->second->value);
	}
}
void printvar_t::array(eharray_t *in) {
	// iterate over strings
	ARRAY_FOR_EACH_STRING(in, i) {
		printf("'%s' => ", i->first.c_str());
		this->retval(i->second);	
	}
	// and ints
	ARRAY_FOR_EACH_INT(in, i) {
		printf("%d => ", i->first);
		this->retval(i->second);	
	}
}
void printvar_t::tuple(ehtuple_t *in) {
  int size = in->size();
  for(int i = 0; i < size; i++) {
    this->retval(in->get(i));
  }
}

EHLIBFUNC(printvar) {
	printvar_t printer(args, ehi);
	// this function always returns NULL
	return NULL;
}

/*
 * Type checking functions
 */
#define TYPEFUNC(typev) EHLIBFUNC(is_ ## typev) { \
	return ehretval_t::make_bool(args->type() == typev ## _e); \
}

TYPEFUNC(null)
TYPEFUNC(int)
TYPEFUNC(string)
TYPEFUNC(bool)
TYPEFUNC(float)
TYPEFUNC(array)
TYPEFUNC(object)
TYPEFUNC(range)
// get the type of a variable
EHLIBFUNC(get_type) {
	return ehretval_t::make_string(strdup(get_typestring(args->type())));
}

/*
 * Including files
 */
EHLIBFUNC(include) {
	if(args->type() != string_e) {
		ehi->returning = false;
		throw_TypeError("Invalid type for argument to include", args->type(), ehi);
	}
	// do the work
	FILE *infile = fopen(args->get_stringval(), "r");
	if(!infile) {
		ehi->returning = false;
		throw_ArgumentError("Unable to open file", "include", args, ehi);
	}
	EHParser parser(end_is_end_e, ehi, obj);
	ehretval_p parse_return = parser.parse_file(infile);
	// we're no longer returning
	ehi->returning = false;
	return parse_return;
}

// power
EHLIBFUNC(pow) {
	ASSERT_NARGS(2, "pow");
	ehretval_p rhs = args->get_tupleval()->get(0);
	ehretval_p lhs = args->get_tupleval()->get(1);
	if(rhs->type() == int_e) {
		if(lhs->type() == int_e) {
			return ehretval_t::make_int(pow((float) rhs->get_intval(), (float) lhs->get_intval()));
		} else if(lhs->type() == float_e) {
			return ehretval_t::make_float(pow((float) rhs->get_intval(), lhs->get_floatval()));
		} else {
			throw_TypeError("Invalid type for argument to pow", lhs->type(), ehi);
		}
	} else if(rhs->type() == float_e) {
		if(lhs->type() == int_e) {
			return ehretval_t::make_float(pow(rhs->get_floatval(), (float) lhs->get_intval()));
		} else if(lhs->type() == float_e) {
			return ehretval_t::make_float(pow(rhs->get_floatval(), lhs->get_floatval()));
		} else {
			throw_TypeError("Invalid type for argument to pow", lhs->type(), ehi);
		}
	} else {
		throw_TypeError("Invalid type for argument to pow", rhs->type(), ehi);
	}
	return NULL;
}

EHLIBFUNC(log) {
	ehretval_p arg = ehi->to_float(args, obj);
	return ehretval_t::make_float(log(arg->get_floatval()));
}

EHLIBFUNC(eval) {
	ehretval_p arg = ehi->to_string(args, obj);
	return ehi->parse_string(arg->get_stringval(), obj);
}

EHLIBFUNC(getinput) {
	ASSERT_NULL("getinput");
	// more accurately, getint
	ehretval_p ret = ehretval_t::make_typed(int_e);
	fscanf(stdin, "%d", &(ret->intval));
	return ret;
}

EHLIBFUNC(throw) {
	throw eh_exception(args);
}

EHLIBFUNC(echo) {
	ehretval_p str = ehi->to_string(args, obj);
	std::cout << str->get_stringval() << std::endl;
	return NULL;
}

EHLIBFUNC(put) {
	ehretval_p str = ehi->to_string(args, obj);
	std::cout << str->get_stringval();
	return NULL;
}
EHLIBFUNC(collectGarbage) {
	ehi->gc.do_collect(ehi->global_object);
	return NULL;
}
