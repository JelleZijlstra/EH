/*
 * eh_libclasses.cpp
 *
 * EH library classes
 */
#include "eh_libclasses.h"
#include <stdio.h>
#include <cmath>

#define ASSERT_NARGS(count, method) if(nargs != count) { \
	eh_error_argcount_lib(#method, count, nargs); \
	return NULL; \
}
#define ASSERT_TYPE(operand, ehtype, method) if(operand->type() != ehtype) { \
	eh_error_type("argument to " #method, operand->type(), enotice_e); \
	return NULL; \
}
#define ASSERT_OBJ_TYPE(ehtype, method) if(obj->type() != ehtype) { \
	eh_error_type("base object of " #method, obj->type(), enotice_e); \
	return NULL; \
}
#define ASSERT_NARGS_AND_TYPE(count, ehtype, method) ASSERT_NARGS(count, method); ASSERT_OBJ_TYPE(ehtype, method);

START_EHLC(Object)
EHLC_ENTRY(Object, new)
EHLC_ENTRY(Object, initialize)
EHLC_ENTRY(Object, toString)
EHLC_ENTRY(Object, finalize)
END_EHLC()

EH_METHOD(Object, new) {
	ehretval_p ret = ehi->object_instantiate(context->get_objectval(), context);
	ret->get_objectval()->object_data = ehi->call_method(ret, "initialize", nargs, args, ret);
	return ret;
}
EH_METHOD(Object, initialize) {
	return NULL;
}
EH_METHOD(Object, toString) {
	ASSERT_NARGS(0, "Object.toString");
	const size_t len = sizeof(void *) * 2 + 3;
	char *out = new char[len]();
	snprintf(out, len, "%p", reinterpret_cast<void *>(obj.operator->()));
	return ehretval_t::make_string(out);
}
EH_METHOD(Object, finalize) {
	return NULL;
}

START_EHLC(CountClass)
EHLC_ENTRY(CountClass, initialize)
EHLC_ENTRY(CountClass, docount)
EHLC_ENTRY(CountClass, setcount)
END_EHLC()

EH_METHOD(CountClass, initialize) {
	return ehretval_t::make_resource((LibraryBaseClass *)new CountClass());
}
EH_METHOD(CountClass, docount) {
	ASSERT_NARGS(0, "CountClass.docount");
	CountClass *selfptr = (CountClass *)obj->get_resourceval();
	return ehretval_t::make_int(++selfptr->count);
}
EH_METHOD(CountClass, setcount) {
	ASSERT_NARGS(1, "CountClass.setcount");
	CountClass *selfptr = (CountClass *)obj->get_resourceval();
	ehretval_p newcounter = args[0];
	ASSERT_TYPE(newcounter, int_e, "CountClass.setcount");
	selfptr->count = newcounter->get_intval();
	return ehretval_t::make_bool(true);
}

START_EHLC(File)
EHLC_ENTRY(File, initialize)
EHLC_ENTRY(File, open)
EHLC_ENTRY(File, getc)
EHLC_ENTRY(File, gets)
EHLC_ENTRY(File, puts)
EHLC_ENTRY(File, close)
EHLC_ENTRY(File, toBool)
EHLC_ENTRY(File, finalize)
END_EHLC()

EH_METHOD(File, initialize) {
	return ehretval_t::make_resource((LibraryBaseClass *)new File());
}
EH_METHOD(File, open) {
	ASSERT_NARGS(1, "File.open");
	File *selfptr = (File *) obj->get_resourceval();

	ehretval_p filename = ehi->to_string(args[0], context);
	ASSERT_TYPE(filename, string_e, "File.open");
	FILE *mfile = fopen(filename->get_stringval(), "r+");
	if(mfile == NULL) {
		return NULL;
	}
	selfptr->descriptor = mfile;
	return ehretval_t::make_bool(true);
}
EH_METHOD(File, getc) {
	ASSERT_NARGS(0, "File.getc");
	File *selfptr = (File *) obj->get_resourceval();

	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	int c = fgetc(selfptr->descriptor);
	if(c == -1) {
		return NULL;
	}
	char *out = new char[2];
	out[0] = c;
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(File, gets) {
	ASSERT_NARGS(0, "File.gets");
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	
	char *out = new char[512];

	char *ptr = fgets(out, 511, selfptr->descriptor);
	if(ptr == NULL) {
		delete[] out;
		return NULL;
	}
	return ehretval_t::make_string(out);
}
EH_METHOD(File, puts) {
	ASSERT_NARGS(1, "File.puts");
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	
	ehretval_p str = args[0];
	ASSERT_TYPE(str, string_e, "File.puts");

	int count = fputs(str->get_stringval(), selfptr->descriptor);
	
	if(count == EOF) {
		return ehretval_t::make_bool(false);
	} else {
		return ehretval_t::make_bool(true);
	}
}
EH_METHOD(File, close) {
	ASSERT_NARGS(0, "File.close");
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	fclose(selfptr->descriptor);
	selfptr->descriptor = NULL;
	return NULL;
}
EH_METHOD(File, toBool) {
	ASSERT_NARGS(0, "File.toBool");
	File *selfptr = (File *)obj->get_resourceval();
	return ehretval_t::make_bool(selfptr->descriptor != NULL);
}
EH_METHOD(File, finalize) {
	File *selfptr = (File *)obj->get_resourceval();
	if(selfptr->descriptor != NULL) {
		fclose(selfptr->descriptor);
		selfptr->descriptor = NULL;
	}
	return NULL;
}

START_EHLC(Integer)
EHLC_ENTRY(Integer, operator_plus)
EHLC_ENTRY(Integer, operator_minus)
EHLC_ENTRY(Integer, operator_times)
EHLC_ENTRY(Integer, operator_divide)
EHLC_ENTRY(Integer, operator_modulo)
EHLC_ENTRY(Integer, operator_and)
EHLC_ENTRY(Integer, operator_or)
EHLC_ENTRY(Integer, operator_xor)
EHLC_ENTRY(Integer, operator_tilde)
EHLC_ENTRY(Integer, operator_uminus)
EHLC_ENTRY(Integer, abs)
EHLC_ENTRY(Integer, getBit)
EHLC_ENTRY(Integer, setBit)
EHLC_ENTRY(Integer, length)
EHLC_ENTRY(Integer, toString)
EHLC_ENTRY(Integer, toBool)
EHLC_ENTRY(Integer, toFloat)
EHLC_ENTRY(Integer, toInt)
EHLC_ENTRY(Integer, sqrt)
END_EHLC()

EH_METHOD(Integer, operator_plus) {
	ASSERT_NARGS_AND_TYPE(1, int_e, "Integer.operator+");
	ehretval_p operand = args[0];
	if(operand->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() + operand->get_floatval());
	} else {
		operand = ehi->to_int(operand, context);
		if(operand->type() == int_e) {
			return ehretval_t::make_int(obj->get_intval() + operand->get_intval());
		} else {
			eh_error_type("argument to Integer.operator+", args[0]->type(), enotice_e);
			return NULL;
		}
	}
}
EH_METHOD(Integer, operator_minus) {
	ASSERT_NARGS_AND_TYPE(1, int_e, "Integer.operator-");
	ehretval_p operand = args[0];
	if(operand->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() - operand->get_floatval());
	} else {
		operand = ehi->to_int(operand, context);
		if(operand->type() == int_e) {
			return ehretval_t::make_int(obj->get_intval() - operand->get_intval());
		} else {
			eh_error_type("argument to Integer.operator-", args[0]->type(), enotice_e);
			return NULL;
		}
	}
}
EH_METHOD(Integer, operator_times) {
	ASSERT_NARGS_AND_TYPE(1, int_e, "Integer.operator*");
	ehretval_p operand = args[0];
	if(operand->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() * operand->get_floatval());
	} else {
		operand = ehi->to_int(operand, context);
		if(operand->type() == int_e) {
			return ehretval_t::make_int(obj->get_intval() * operand->get_intval());
		} else {
			eh_error_type("argument to Integer.operator*", args[0]->type(), enotice_e);
			return NULL;
		}
	}
}
EH_METHOD(Integer, operator_divide) {
	ASSERT_NARGS_AND_TYPE(1, int_e, "Integer.operator/");
	ehretval_p operand = args[0];
	if(operand->type() == float_e) {
		float val = operand->get_floatval();
		if(val == 0.0) {
			eh_error("Divide by zero", enotice_e);
			return NULL;
		}
		return ehretval_t::make_float((float) obj->get_intval() / val);
	} else {
		operand = ehi->to_int(operand, context);
		if(operand->type() == int_e) {
			int val = operand->get_intval();
			if(val == 0) {
				eh_error("Divide by zero", enotice_e);
				return NULL;
			}
			return ehretval_t::make_int(obj->get_intval() / operand->get_intval());
		} else {
			eh_error_type("argument to Integer.operator/", args[0]->type(), enotice_e);
			return NULL;
		}
	}
}
EH_METHOD(Integer, operator_modulo) {
	ASSERT_NARGS_AND_TYPE(1, int_e, "Integer.operator%");
	ehretval_p operand = ehi->to_int(args[0], context);
	ASSERT_TYPE(operand, int_e, "Integer.operator%");
	if(operand->get_intval() == 0) {
		eh_error("Divide by zero", enotice_e);
		return NULL;
	}
	return ehretval_t::make_int(obj->get_intval() % operand->get_intval());
}
EH_METHOD(Integer, operator_and) {
  ASSERT_NARGS_AND_TYPE(1, int_e, "Integer.operator&");
  ehretval_p operand = args[0];
  ASSERT_TYPE(operand, int_e, "Integer.operator&");
  return ehretval_t::make_int(obj->get_intval() & operand->get_intval());
}
EH_METHOD(Integer, operator_or) {
  ASSERT_NARGS_AND_TYPE(1, int_e, "integer.operator|");
  ehretval_p operand = args[0];
  ASSERT_TYPE(operand, int_e, "Integer.operator|");
  return ehretval_t::make_int(obj->get_intval() | operand->get_intval());
}
EH_METHOD(Integer, operator_xor) {
  ASSERT_NARGS_AND_TYPE(1, int_e, "integer.operator^");
  ehretval_p operand = args[0];
  ASSERT_TYPE(operand, int_e, "Integer.operator^");
  return ehretval_t::make_int(obj->get_intval() ^ operand->get_intval());
}
EH_METHOD(Integer, operator_tilde) {
  ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.operator~");
  return ehretval_t::make_int(~obj->get_intval());
}
EH_METHOD(Integer, operator_uminus) {
  ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.operator-");
  return ehretval_t::make_int(-obj->get_intval());
}
EH_METHOD(Integer, abs) {
	ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.abs");
	return ehretval_t::make_int(abs(obj->get_intval()));
}
EH_METHOD(Integer, getBit) {
	ASSERT_NARGS_AND_TYPE(1, int_e, "Integer.getBit");
	ehretval_p operand = args[0];
	ASSERT_TYPE(operand, int_e, "Integer.getBit");
	int index = operand->get_intval();
	if(index < 0 || index >= sizeof(int) * 8) {
		eh_error_invalid_argument("Integer.getBit", 0);
		return NULL;
	}
	// get mask
	int mask = 1 << (sizeof(int) * 8 - 1);
	mask >>= index;
	// apply mask
	return ehretval_t::make_bool((obj->get_intval() & mask) >> (sizeof(int) * 8 - 1 - mask));
}
EH_METHOD(Integer, setBit) {
	ASSERT_NARGS_AND_TYPE(2, int_e, "Integer.setBit");
	ehretval_p operand = args[0];
	ASSERT_TYPE(operand, int_e, "Integer.setBit");
	int index = operand->get_intval();
	if(index < 0 || index >= sizeof(int) * 8) {
		eh_error_invalid_argument("Integer.setBit", 0);
		return NULL;
	}
	int new_value;
	ehretval_p value = args[1];
	if(value->type() == int_e) {
		int int_value = value->get_intval();
		if(int_value != 0 && int_value != 1) {
			eh_error_invalid_argument("Integer.setBit", 1);
			return NULL;
		}
		new_value = int_value;
	} else if(value->type() == bool_e) {
		new_value = value->get_boolval();
	} else {
		eh_error_type("argument to Integer.setBit", value->type(), enotice_e);
		return NULL;
	}
	int mask = (1 << (sizeof(int) * 8 - 1)) >> index;
	int out = obj->get_intval();
	if(new_value) {
		out |= mask;
	} else {
		mask = ~mask;
		out &= mask;
	}
	return ehretval_t::make_int(out);
}
EH_METHOD(Integer, length) {
	ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.length");
	return ehretval_t::make_int(sizeof(int));
}
EH_METHOD(Integer, toString) {
	ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.toString");
	// INT_MAX has 10 decimal digits on this computer, so 12 (including sign and
	// null terminator) should suffice for the result string
	char *buffer = new char[12]();
	snprintf(buffer, 12, "%d", obj->get_intval());
	return ehretval_t::make_string(buffer);
}
EH_METHOD(Integer, toBool) {
	ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.toBool");
	return ehretval_t::make_bool(obj->get_intval() != 0);
}
EH_METHOD(Integer, toFloat) {
	ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.toFloat");
	return ehretval_t::make_float((float) obj->get_intval());
}
EH_METHOD(Integer, toInt) {
	ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.toInt");
	return obj;
}
EH_METHOD(Integer, sqrt) {
  ASSERT_NARGS_AND_TYPE(0, int_e, "Integer.sqrt");
  return ehretval_t::make_int((int) sqrt((double) obj->get_intval()));
}

START_EHLC(Array)
EHLC_ENTRY(Array, length)
EHLC_ENTRY(Array, operator_arrow)
EHLC_ENTRY(Array, operator_arrow_equals)
END_EHLC()

EH_METHOD(Array, length) {
	ASSERT_NARGS_AND_TYPE(0, array_e, "Array.length");
	return ehretval_t::make_int(obj->get_arrayval()->size());
}
EH_METHOD(Array, operator_arrow) {
	ASSERT_NARGS_AND_TYPE(1, array_e, "Array.operator->");
	eharray_t *arr = obj->get_arrayval();
	if(arr->has(args[0])) {
		return arr->operator[](args[0]);
	} else {
		return NULL;
	}
}
EH_METHOD(Array, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, array_e, "Array.operator->=");
	try {
		obj->get_arrayval()->operator[](args[0]) = args[1];
		return args[1];
	} catch(unknown_value_exception &e) {
		return NULL;
	}
}

START_EHLC(Float)
EHLC_ENTRY(Float, operator_plus)
EHLC_ENTRY(Float, operator_minus)
EHLC_ENTRY(Float, operator_times)
EHLC_ENTRY(Float, operator_divide)
EHLC_ENTRY(Float, operator_uminus)
EHLC_ENTRY(Float, abs)
EHLC_ENTRY(Float, toString)
EHLC_ENTRY(Float, toInt)
EHLC_ENTRY(Float, toBool)
EHLC_ENTRY(Float, toFloat)
EHLC_ENTRY(Float, sqrt)
END_EHLC()

EH_METHOD(Float, operator_plus) {
	ASSERT_NARGS_AND_TYPE(1, float_e, "Float.operator+");
	ehretval_p operand = ehi->to_float(args[0], context);
	if(operand->type() == float_e) {
		return ehretval_t::make_float(obj->get_floatval() + operand->get_floatval());
	} else {
		eh_error_type("argument to Float.operator+", args[0]->type(), enotice_e);
		return NULL;
	}
}
EH_METHOD(Float, operator_minus) {
	ASSERT_NARGS_AND_TYPE(1, float_e, "Float.operator-");
	ehretval_p operand = ehi->to_float(args[0], context);
	if(operand->type() == float_e) {
		return ehretval_t::make_float(obj->get_floatval() - operand->get_floatval());
	} else {
		eh_error_type("argument to Float.operator-", args[0]->type(), enotice_e);
		return NULL;
	}
}
EH_METHOD(Float, operator_times) {
	ASSERT_NARGS_AND_TYPE(1, float_e, "Float.operator*");
	ehretval_p operand = ehi->to_float(args[0], context);
	if(operand->type() == float_e) {
		return ehretval_t::make_float(obj->get_floatval() * operand->get_floatval());
	} else {
		eh_error_type("argument to Float.operator*", args[0]->type(), enotice_e);
		return NULL;
	}
}
EH_METHOD(Float, operator_divide) {
	ASSERT_NARGS_AND_TYPE(1, float_e, "Float.operator/");
	ehretval_p operand = ehi->to_float(args[0], context);
	if(operand->type() == float_e) {
		if(operand->get_floatval() == 0.0) {
			eh_error("Divide by zero", enotice_e);
			return NULL;
		}
		return ehretval_t::make_float(obj->get_floatval() / operand->get_floatval());
	} else {
		eh_error_type("argument to Float.operator/", args[0]->type(), enotice_e);
		return NULL;
	}
}
EH_METHOD(Float, operator_uminus) {
  ASSERT_NARGS_AND_TYPE(0, float_e, "Float.operator-");
  return ehretval_t::make_float(-obj->get_floatval());
}
EH_METHOD(Float, abs) {
	ASSERT_NARGS_AND_TYPE(0, float_e, "Float.abs");
	return ehretval_t::make_float(abs(obj->get_floatval()));
}
EH_METHOD(Float, toString) {
	ASSERT_NARGS_AND_TYPE(0, float_e, "Float.toString");
	char *buffer = new char[12];
	snprintf(buffer, 12, "%f", obj->get_floatval());
	return ehretval_t::make_string(buffer);
}
EH_METHOD(Float, toInt) {
	ASSERT_NARGS_AND_TYPE(0, float_e, "Float.toInt");
	return ehretval_t::make_int((int) obj->get_floatval());
}
EH_METHOD(Float, toBool) {
	ASSERT_NARGS_AND_TYPE(0, float_e, "Float.toBool");
	return ehretval_t::make_bool(obj->get_floatval() != 0.0);
}
EH_METHOD(Float, toFloat) {
	ASSERT_NARGS_AND_TYPE(0, float_e, "Float.toFloat");
	return obj;
}
EH_METHOD(Float, sqrt) {
  ASSERT_NARGS_AND_TYPE(0, float_e, "Float.sqrt");
  return ehretval_t::make_float(sqrt(obj->get_floatval()));
}

START_EHLC(String)
EHLC_ENTRY(String, operator_plus)
EHLC_ENTRY(String, operator_arrow)
EHLC_ENTRY(String, operator_arrow_equals)
EHLC_ENTRY(String, length)
EHLC_ENTRY(String, toString)
EHLC_ENTRY(String, toInt)
EHLC_ENTRY(String, toFloat)
EHLC_ENTRY(String, toBool)
EHLC_ENTRY(String, toRange)
EHLC_ENTRY(String, charAtPosition)
END_EHLC()

EH_METHOD(String, operator_plus) {
	ASSERT_NARGS_AND_TYPE(1, string_e, "String.operator+");
	ehretval_p operand = ehi->to_string(args[0], context);
	ASSERT_TYPE(operand, string_e, "String.operator+");
	size_t len1 = strlen(obj->get_stringval());
	size_t len2 = strlen(operand->get_stringval());
	char *out = new char[len1 + len2 + 1];
	strcpy(out, obj->get_stringval());
	strcpy(out + len1, operand->get_stringval());
	return ehretval_t::make_string(out); 
}
EH_METHOD(String, operator_arrow) {
	ASSERT_NARGS_AND_TYPE(1, string_e, "String.operator->");
	ehretval_p operand = args[0];
	ASSERT_TYPE(operand, int_e, "String.operator->");
	int index = operand->get_intval();
	size_t len = strlen(obj->get_stringval());
	// allow negative index
	if(index < 0) {
		index += len;
	}
	if(index < 0 || index >= len) {
		eh_error_invalid_argument("String.operator->", 0);
		return NULL;
	}
	char *out = new char[2]();
	out[0] = obj->get_stringval()[index];
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(String, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, string_e, "String.operator->=");
	ehretval_p operand1 = args[0];
	ASSERT_TYPE(operand1, int_e, "String.operator->=");
	int index = operand1->get_intval();
	size_t len = strlen(obj->get_stringval());
	if(index < 0) {
		index += len;
	}
	if(index < 0 || index >= len) {
		eh_error_invalid_argument("String.operator->=", 0);
		return NULL;
	}
	ehretval_p operand2 = ehi->to_string(args[1], context);
	if(strlen(operand2->get_stringval()) == 0) {
		eh_error_invalid_argument("String.operator->=", 1);
		return NULL;
	}
	obj->get_stringval()[index] = operand2->get_stringval()[0];
	return operand2;
}
EH_METHOD(String, length) {
	ASSERT_NARGS_AND_TYPE(0, string_e, "String.length");
	return ehretval_t::make_int(strlen(obj->get_stringval()));
}
EH_METHOD(String, toString) {
	ASSERT_NARGS_AND_TYPE(0, string_e, "String.toString");
	return obj;
}
EH_METHOD(String, toInt) {
	ASSERT_NARGS_AND_TYPE(0, string_e, "String.toInt");
	char *endptr;
	ehretval_p ret = ehretval_t::make_int(strtol(obj->get_stringval(), &endptr, 0));
	// If in == endptr, strtol read no digits and there was no conversion.
	if(obj->get_stringval() == endptr) {
		eh_error("Cannot convert String to Integer", enotice_e);
		return NULL;
	}
	return ret;
}
EH_METHOD(String, toFloat) {
	ASSERT_NARGS_AND_TYPE(0, string_e, "String.toFloat");
	char *endptr;
	ehretval_p ret = ehretval_t::make_float(strtof(obj->get_stringval(), &endptr));
	// If in == endptr, strtof read no digits and there was no conversion.
	if(obj->get_stringval() == endptr) {
		eh_error("Cannot convert String to Float", enotice_e);
		return NULL;
	}
	return ret;
}
EH_METHOD(String, toBool) {
	ASSERT_NARGS_AND_TYPE(0, string_e, "String.toBool");
	return ehretval_t::make_bool(obj->get_stringval()[0] != '\0');
}
EH_METHOD(String, toRange) {
	ASSERT_NARGS_AND_TYPE(0, string_e, "String.toRange");
	// attempt to find two integers in the string
	int min, max;
	char *in = obj->get_stringval();
	char *ptr;
	// get lower part of range
	for(int i = 0; ; i++) {
		if(in[i] == '\0') {
			eh_error("Could not convert string to range", enotice_e);
			return NULL;
		}
		if(isdigit(in[i])) {
			min = strtol(&in[i], &ptr, 0);
			break;
		}
	}
	// get upper bound
	for(int i = 0; ; i++) {
		if(ptr[i] == '\0') {
			eh_error("Could not convert string to range", enotice_e);
			return NULL;
		}
		if(isdigit(ptr[i])) {
			max = strtol(&ptr[i], NULL, 0);
			break;
		}
	}
	ehrange_t *range = new ehrange_t(ehretval_t::make_int(min), ehretval_t::make_int(max));
	return ehi->make_range(range);
}
EH_METHOD(String, charAtPosition) {
	ASSERT_NARGS_AND_TYPE(1, string_e, "String.charAtPosition");
	ehretval_p index_var = args[0];
	ASSERT_TYPE(index_var, int_e, "String.charAtPosition");
	int index = index_var->get_intval();
	const char *string = obj->get_stringval();
	if(index < 0 || index >= strlen(string)) {
		eh_error_invalid_argument("String.charAtPosition", index);
		return NULL;
	} else {
		return ehretval_t::make_int(string[index]);
	}
}

START_EHLC(Bool)
EHLC_ENTRY(Bool, toString)
EHLC_ENTRY(Bool, toBool)
EHLC_ENTRY(Bool, toInt)
EHLC_ENTRY(Bool, operator_bang)
END_EHLC()

EH_METHOD(Bool, toString) {
	ASSERT_NARGS_AND_TYPE(0, bool_e, "Bool.toString");
	char *str;
	if(obj->get_boolval()) {
		str = strdup("true");
	} else {
		str = strdup("false");
	}
	return ehretval_t::make_string(str);
}
EH_METHOD(Bool, toBool) {
	ASSERT_NARGS_AND_TYPE(0, bool_e, "Bool.toBool");
	return obj;
}
EH_METHOD(Bool, toInt) {
	ASSERT_NARGS_AND_TYPE(0, bool_e, "Bool.toInt");
	if(obj->get_boolval()) {
		return ehretval_t::make_int(1);
	} else {
		return ehretval_t::make_int(0);
	}
}
EH_METHOD(Bool, operator_bang) {
  ASSERT_NARGS_AND_TYPE(0, bool_e, "Bool.operator!");
  return ehretval_t::make_bool(!obj->get_boolval());
}

START_EHLC(Null)
EHLC_ENTRY(Null, toString)
EHLC_ENTRY(Null, toBool)
END_EHLC()

EH_METHOD(Null, toString) {
	ASSERT_NARGS_AND_TYPE(0, null_e, "Null.toString");
	return ehretval_t::make_string(strdup(""));
}
EH_METHOD(Null, toBool) {
	ASSERT_NARGS_AND_TYPE(0, null_e, "Null.toBool");
	return ehretval_t::make_bool(false);
}

START_EHLC(Range)
EHLC_ENTRY(Range, min)
EHLC_ENTRY(Range, max)
EHLC_ENTRY(Range, operator_arrow)
EHLC_ENTRY(Range, toString)
EHLC_ENTRY(Range, toArray)
EHLC_ENTRY(Range, toRange)
END_EHLC()

EH_METHOD(Range, min) {
	ASSERT_NARGS_AND_TYPE(0, range_e, "Range.min");
	return obj->get_rangeval()->min;
}
EH_METHOD(Range, max) {
	ASSERT_NARGS_AND_TYPE(0, range_e, "Range.max");
	return obj->get_rangeval()->max;
}
EH_METHOD(Range, operator_arrow) {
	ASSERT_NARGS_AND_TYPE(1, range_e, "Range.operator->");
	ehretval_p operand = args[0];
	ASSERT_TYPE(operand, int_e, "Range.operator->");
	int index = operand->get_intval();
	if(index == 0) {
		return obj->get_rangeval()->min;
	} else if(index == 1) {
		return obj->get_rangeval()->max;
	} else {
		eh_error_invalid_argument("Range.operator->", 0);
		return NULL;		
	}
}
EH_METHOD(Range, toString) {
	ASSERT_NARGS_AND_TYPE(0, range_e, "Range.toString");
	ehrange_t *range = obj->get_rangeval();
	ehretval_p str1 = ehi->to_string(range->min, context);
	ehretval_p str2 = ehi->to_string(range->max, context);
	size_t len1 = strlen(str1->get_stringval());
	size_t len2 = strlen(str2->get_stringval());
	size_t len = len1 + 4 + len2 + 1;
	char *out = new char[len]();
	strncpy(out, str1->get_stringval(), len1);
	strncpy(out + len1, " to ", 4);
	strncpy(out + len1 + 4, str2->get_stringval(), len2);
	out[len1 + 4 + len2] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(Range, toArray) {
	ASSERT_NARGS_AND_TYPE(0, range_e, "Range.toArray");
	ehretval_p array = ehi->make_array(new eharray_t);
	array->get_arrayval()->int_indices[0] = obj->get_rangeval()->min;
	array->get_arrayval()->int_indices[1] = obj->get_rangeval()->max;
	return array;
}
EH_METHOD(Range, toRange) {
	ASSERT_NARGS_AND_TYPE(0, range_e, "Range.toRange");
	return obj;
}

START_EHLC(Hash)
EHLC_ENTRY(Hash, toArray)
EHLC_ENTRY(Hash, operator_arrow)
EHLC_ENTRY(Hash, operator_arrow_equals)
EHLC_ENTRY(Hash, has)
END_EHLC()

EH_METHOD(Hash, toArray) {
	ASSERT_NARGS_AND_TYPE(0, hash_e, "Hash.toArray");
	ehhash_t *hash = obj->get_hashval();
	eharray_t *arr = new eharray_t();
	ehretval_p out = ehi->make_array(arr);
	HASH_FOR_EACH(hash, i) {
		arr->string_indices[i->first.c_str()] = i->second;
	}
	return out;
}
EH_METHOD(Hash, operator_arrow) {
	ASSERT_NARGS_AND_TYPE(1, hash_e, "Hash.operator->");
	ehretval_p index = args[0];
	ASSERT_TYPE(index, string_e, "Hash.operator->");
	ehhash_t *hash = obj->get_hashval();
	return hash->get(index->get_stringval());
}
EH_METHOD(Hash, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, hash_e, "Hash.operator->=");
	ehretval_p index = args[0];
	ASSERT_TYPE(index, string_e, "Hash.operator->=");
	ehretval_p value = args[1];
	ehhash_t *hash = obj->get_hashval();
	hash->set(index->get_stringval(), value);
	return value;
}
EH_METHOD(Hash, has) {
	ASSERT_NARGS_AND_TYPE(1, hash_e, "Hash.has");
	ehretval_p index = args[0];
	ASSERT_TYPE(index, string_e, "Hash.has");
	ehhash_t *hash = obj->get_hashval();
	return ehretval_t::make_bool(hash->has(index->get_stringval()));
}

START_EHLC(Function)
EHLC_ENTRY(Function, operator_colon)
END_EHLC()

EH_METHOD(Function, operator_colon) {
	// This is probably the most important library method in EH. It works
	// on both Function and binding objects.
	ehretval_p object_data;
	ehretval_p function_object;
	ehretval_p parent;
	ehretval_p real_parent;
	if(context->is_a(func_e)) {
		function_object = context;
		parent = context->get_objectval()->parent;
		real_parent = context->get_objectval()->real_parent;
		object_data = context->get_objectval()->parent->get_objectval()->object_data;
	} else if(context->type() == binding_e) {
		ehbinding_t *binding = context->get_bindingval();
		function_object = binding->method;
		parent = binding->method->get_objectval()->parent;
		real_parent = binding->method->get_objectval()->real_parent;
		object_data = binding->object_data;
	} else {
		eh_error_type("base object of Function.operator:", context->type(), enotice_e);
		return NULL;
	}
	ehfunc_t *f = function_object->get_objectval()->object_data->get_funcval();

	if(f->type == lib_e) {
		return f->libmethod_pointer(object_data, nargs, args, parent, ehi);
	}
	// check parameter count
	if(nargs != f->argcount) {
		eh_error_argcount(f->argcount, nargs);
		return NULL;
	}
	ehretval_p newcontext = ehi->object_instantiate(function_object->get_objectval(), context);
	newcontext->get_objectval()->object_data = function_object->get_objectval()->object_data;
	
	// set parameters as necessary
	for(int i = 0; i < nargs; i++) {
		ehmember_p var;
		var->value = args[i];
		newcontext->get_objectval()->insert(f->args[i].name, var);
	}
	// insert self variable with the object_data
	ehmember_p self_member;
	self_member->value = object_data;
	newcontext->get_objectval()->insert("self", self_member);
	
	ehretval_p ret = ehi->eh_execute(f->code, newcontext);
	ehi->returning = false;
	return ret;
}

START_EHLC(Exception)
EHLC_ENTRY(Exception, initialize)
EHLC_ENTRY(Exception, toString)
END_EHLC()

EH_METHOD(Exception, initialize) {
	ASSERT_NARGS(1, "Exception.initialize");
	ehretval_p msg = args[0];
	ASSERT_TYPE(msg, string_e, "Exception.initialize");
	return ehretval_t::make_resource(new Exception(msg->get_stringval()));
}
EH_METHOD(Exception, toString) {
	ASSERT_NARGS_AND_TYPE(0, resource_e, "Exception.toString");
	Exception *exc = (Exception *)obj->get_resourceval();
	return ehretval_t::make_string(strdup(exc->msg));
}
