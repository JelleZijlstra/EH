/*
 * eh_libclasses.cpp
 *
 * EH library classes
 */
#include "eh_libclasses.h"
#include <stdio.h>
#include <cmath>
#include <sstream>

// Helpers
static int intcmp(int lhs, int rhs) {
  if(lhs < rhs) {
    return -1;
  } else if(lhs == rhs) {
    return 0;
  } else {
    return 1;
  }	
}

START_EHLC(Object)
EHLC_ENTRY(Object, new)
EHLC_ENTRY(Object, inherit)
EHLC_ENTRY(Object, initialize)
EHLC_ENTRY(Object, toString)
EHLC_ENTRY(Object, finalize)
EHLC_ENTRY(Object, isA)
EHLC_ENTRY_RENAME(Object, operator_compare, "operator<=>")
EHLC_ENTRY(Object, compare)
EHLC_ENTRY_RENAME(Object, operator_equals, "operator==")
EHLC_ENTRY_RENAME(Object, operator_ne, "operator!=")
EHLC_ENTRY_RENAME(Object, operator_gt, "operator>")
EHLC_ENTRY_RENAME(Object, operator_gte, "operator>=")
EHLC_ENTRY_RENAME(Object, operator_lt, "operator<")
EHLC_ENTRY_RENAME(Object, operator_lte, "operator<=")
END_EHLC()

EH_METHOD(Object, new) {
	ehretval_p ret = ehi->object_instantiate(context->get_objectval());
	ret->get_objectval()->object_data = ehi->call_method(ret, "initialize", args, ret);
	return ret;
}
EH_METHOD(Object, inherit) {
  	ASSERT_TYPE(args, object_e, "Object.inherit");
	ehobj_t *classobj = args->get_objectval();
	if(classobj != NULL) {
		OBJECT_FOR_EACH(classobj, i) {
			context->get_objectval()->copy_member(i, true, context, ehi);
		}
	}
	return NULL;
}
EH_METHOD(Object, initialize) {
	return NULL;
}
EH_METHOD(Object, toString) {
	ASSERT_TYPE(args, null_e, "Object.toString");
	const size_t len = sizeof(void *) * 2 + 3;
	char *out = new char[len]();
	snprintf(out, len, "%p", reinterpret_cast<void *>(obj.operator->()));
	return ehretval_t::make_string(out);
}
EH_METHOD(Object, finalize) {
	return NULL;
}
EH_METHOD(Object, isA) {
  int type = args->type();
  if(type == object_e) {
    type = args->get_objectval()->type_id;
  }
  return ehretval_t::make_bool(context->is_a(type));
}

ehretval_p get_data(ehretval_p in) {
	if(in->is_object()) {
		return in->get_objectval()->object_data;
	} else {
		return in;
	}

}
EH_METHOD(Object, operator_compare) {
	int lhs_type = context->get_full_type();
	int rhs_type = args->get_full_type();
	int comparison = intcmp(lhs_type, rhs_type);
	if(comparison != 0) {
		return ehretval_t::make_int(comparison);
	} else {
		return ehi->call_method_from_method(obj, context, "compare", args);
	}
}
EH_METHOD(Object, compare) {
	int lhs_type = context->get_full_type();
	int rhs_type = args->get_full_type();
	if(lhs_type != rhs_type) {
		throw_TypeError("Arguments to Object.compare must have the same type", rhs_type, ehi);
	}
	ehretval_p lhs = get_data(context);
	if(lhs->type() == null_e) {
		lhs = obj;
	}
	ehretval_p rhs = get_data(args);
	return ehretval_t::make_int(lhs->naive_compare(rhs));	
}
#define CALL_COMPARE() \
	ehretval_p comparison_p = ehi->call_method_from_method(obj, context, "operator<=>", args); \
	if(comparison_p->type() != int_e) { \
		throw_TypeError("operator<=> must return an Integer", comparison_p->type(), ehi); \
	} \
	int comparison = comparison_p->get_intval();

EH_METHOD(Object, operator_equals) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison == 0);
}
EH_METHOD(Object, operator_ne) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison != 0);
}
EH_METHOD(Object, operator_gt) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison == 1);
}
EH_METHOD(Object, operator_gte) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison != -1);
}
EH_METHOD(Object, operator_lt) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison == -1);
}
EH_METHOD(Object, operator_lte) {
	CALL_COMPARE();
	return ehretval_t::make_bool(comparison != 1);
}

// type for the global object (currently empty; ultimately library methods should go here)
START_EHLC(GlobalObject)
EHLC_ENTRY(GlobalObject, toString)
END_EHLC()

EH_METHOD(GlobalObject, toString) {
	return ehretval_t::make_string(strdup("(global execution context)"));
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
	ASSERT_TYPE(args, null_e, "CountClass.docount");
	CountClass *selfptr = (CountClass *)obj->get_resourceval();
	return ehretval_t::make_int(++selfptr->count);
}
EH_METHOD(CountClass, setcount) {
	CountClass *selfptr = (CountClass *)obj->get_resourceval();
	ASSERT_TYPE(args, int_e, "CountClass.setcount");
	selfptr->count = args->get_intval();
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
	File *selfptr = (File *) obj->get_resourceval();

	ehretval_p filename = ehi->to_string(args, context);
	ASSERT_TYPE(filename, string_e, "File.open");
	FILE *mfile = fopen(filename->get_stringval(), "r+");
	if(mfile == NULL) {
		return NULL;
	}
	selfptr->descriptor = mfile;
	return ehretval_t::make_bool(true);
}
EH_METHOD(File, getc) {
	ASSERT_TYPE(args, null_e, "File.getc");
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
	ASSERT_TYPE(args, null_e, "File.gets");
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
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	
	ASSERT_TYPE(args, string_e, "File.puts");

	int count = fputs(args->get_stringval(), selfptr->descriptor);
	
	if(count == EOF) {
		return ehretval_t::make_bool(false);
	} else {
		return ehretval_t::make_bool(true);
	}
}
EH_METHOD(File, close) {
	ASSERT_TYPE(args, null_e, "File.close");
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	fclose(selfptr->descriptor);
	selfptr->descriptor = NULL;
	return NULL;
}
EH_METHOD(File, toBool) {
	ASSERT_TYPE(args, null_e, "File.toBool");
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
EHLC_ENTRY_RENAME(Integer, operator_plus, "operator+")
EHLC_ENTRY_RENAME(Integer, operator_minus, "operator-")
EHLC_ENTRY_RENAME(Integer, operator_times, "operator*")
EHLC_ENTRY_RENAME(Integer, operator_divide, "operator/")
EHLC_ENTRY_RENAME(Integer, operator_modulo, "operator%")
EHLC_ENTRY_RENAME(Integer, operator_and, "operator&")
EHLC_ENTRY_RENAME(Integer, operator_or, "operator|")
EHLC_ENTRY_RENAME(Integer, operator_xor, "operator^")
EHLC_ENTRY_RENAME(Integer, operator_tilde, "operator~")
EHLC_ENTRY_RENAME(Integer, operator_leftshift, "operator<<")
EHLC_ENTRY_RENAME(Integer, operator_rightshift, "operator>>")
EHLC_ENTRY(Integer, operator_uminus)
EHLC_ENTRY(Integer, compare)
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

EH_METHOD(Integer, initialize) {
	return ehi->to_int(args, context);
}
EH_METHOD(Integer, operator_plus) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator+");
	if(args->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() + args->get_floatval());
	} else {
		// always returns an int or throws
		args = ehi->to_int(args, context);
		return ehretval_t::make_int(obj->get_intval() + args->get_intval());
	}
}
EH_METHOD(Integer, operator_minus) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator-");
	if(args->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() - args->get_floatval());
	} else {
		args = ehi->to_int(args, context);
		return ehretval_t::make_int(obj->get_intval() - args->get_intval());
	}
}
EH_METHOD(Integer, operator_times) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator*");
	if(args->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() * args->get_floatval());
	} else {
		args = ehi->to_int(args, context);
		return ehretval_t::make_int(obj->get_intval() * args->get_intval());
	}
}
EH_METHOD(Integer, operator_divide) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator/");
	if(args->type() == float_e) {
		float val = args->get_floatval();
		if(val == 0.0) {
			throw_MiscellaneousError("Divide by zero in Integer.operator/", ehi);
		}
		return ehretval_t::make_float((float) obj->get_intval() / val);
	} else {
		args = ehi->to_int(args, context);
		int val = args->get_intval();
		if(val == 0) {
			throw_MiscellaneousError("Divide by zero in Integer.operator/", ehi);
		}
		return ehretval_t::make_int(obj->get_intval() / args->get_intval());
	}
}
EH_METHOD(Integer, operator_modulo) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator%");
	ehretval_p operand = ehi->to_int(args, context);
	ASSERT_TYPE(operand, int_e, "Integer.operator%");
	if(operand->get_intval() == 0) {
		throw_MiscellaneousError("Divide by zero in Integer.operator%", ehi);
	}
	return ehretval_t::make_int(obj->get_intval() % operand->get_intval());
}
EH_METHOD(Integer, operator_and) {
  ASSERT_OBJ_TYPE(int_e, "Integer.operator&");
  ASSERT_TYPE(args, int_e, "Integer.operator&");
  return ehretval_t::make_int(obj->get_intval() & args->get_intval());
}
EH_METHOD(Integer, operator_or) {
  ASSERT_OBJ_TYPE(int_e, "Integer.operator|");
  ASSERT_TYPE(args, int_e, "Integer.operator|");
  return ehretval_t::make_int(obj->get_intval() | args->get_intval());
}
EH_METHOD(Integer, operator_xor) {
  ASSERT_OBJ_TYPE(int_e, "Integer.operator^");
  ASSERT_TYPE(args, int_e, "Integer.operator^");
  return ehretval_t::make_int(obj->get_intval() ^ args->get_intval());
}
EH_METHOD(Integer, operator_tilde) {
  ASSERT_NULL_AND_TYPE(int_e, "Integer.operator~");
  return ehretval_t::make_int(~obj->get_intval());
}
EH_METHOD(Integer, operator_uminus) {
  ASSERT_NULL_AND_TYPE(int_e, "Integer.operator-");
  return ehretval_t::make_int(-obj->get_intval());
}
EH_METHOD(Integer, compare) {
	ASSERT_OBJ_TYPE(int_e, "Integer.compare");
	ASSERT_TYPE(args, int_e, "Integer.compare");
	return ehretval_t::make_int(intcmp(obj->get_intval(), args->get_intval()));
}
EH_METHOD(Integer, operator_leftshift) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator<<");
	ASSERT_TYPE(args, int_e, "Integer.operator<<");
	return ehretval_t::make_int(obj->get_intval() << args->get_intval());
}
EH_METHOD(Integer, operator_rightshift) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator>>");
	ASSERT_TYPE(args, int_e, "Integer.operator>>");
	return ehretval_t::make_int(obj->get_intval() >> args->get_intval());
}
EH_METHOD(Integer, abs) {
	ASSERT_NULL_AND_TYPE(int_e, "Integer.abs");
	return ehretval_t::make_int(abs(obj->get_intval()));
}
EH_METHOD(Integer, getBit) {
	ASSERT_OBJ_TYPE(int_e, "Integer.getBit");
	ASSERT_TYPE(args, int_e, "Integer.getBit");
	int index = args->get_intval();
	if(index < 0 || ((unsigned) index) >= sizeof(int) * 8) {
		throw_ArgumentError_out_of_range("Integer.getBit", args, ehi);
	}
	// get mask
	int mask = 1 << (sizeof(int) * 8 - 1);
	mask >>= index;
	// apply mask
	return ehretval_t::make_bool((obj->get_intval() & mask) >> (sizeof(int) * 8 - 1 - mask));
}
EH_METHOD(Integer, setBit) {
	ASSERT_NARGS_AND_TYPE(2, int_e, "Integer.setBit");
	ehretval_p operand = args->get_tupleval()->get(0);
	ASSERT_TYPE(operand, int_e, "Integer.setBit");
	int index = operand->get_intval();
	if(index < 0 || ((unsigned) index) >= sizeof(int) * 8) {
		throw_ArgumentError_out_of_range("Integer.setBit", operand, ehi);
	}
	int new_value = 0;
	ehretval_p value = args->get_tupleval()->get(1);
	if(value->type() == int_e) {
		int int_value = value->get_intval();
		if(int_value != 0 && int_value != 1) {
			throw_ArgumentError_out_of_range("Integer.setBit", value, ehi);
		}
		new_value = int_value;
	} else if(value->type() == bool_e) {
		new_value = value->get_boolval();
	} else {
		throw_TypeError("Second argument to Integer.setBit must be an Integer or Bool", value->type(), ehi);
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
	ASSERT_NULL_AND_TYPE(int_e, "Integer.length");
	return ehretval_t::make_int(sizeof(int));
}
EH_METHOD(Integer, toString) {
	ASSERT_NULL_AND_TYPE(int_e, "Integer.toString");
	// INT_MAX has 10 decimal digits on this computer, so 12 (including sign and
	// null terminator) should suffice for the result string
	char *buffer = new char[12]();
	snprintf(buffer, 12, "%d", obj->get_intval());
	return ehretval_t::make_string(buffer);
}
EH_METHOD(Integer, toBool) {
	ASSERT_NULL_AND_TYPE(int_e, "Integer.toBool");
	return ehretval_t::make_bool(obj->get_intval() != 0);
}
EH_METHOD(Integer, toFloat) {
	ASSERT_NULL_AND_TYPE(int_e, "Integer.toFloat");
	return ehretval_t::make_float((float) obj->get_intval());
}
EH_METHOD(Integer, toInt) {
	ASSERT_NULL_AND_TYPE(int_e, "Integer.toInt");
	return obj;
}
EH_METHOD(Integer, sqrt) {
  ASSERT_NULL_AND_TYPE(int_e, "Integer.sqrt");
  return ehretval_t::make_int((int) sqrt((double) obj->get_intval()));
}

START_EHLC(Array)
EHLC_ENTRY(Array, initialize)
EHLC_ENTRY(Array, length)
EHLC_ENTRY_RENAME(Array, operator_arrow, "operator->")
EHLC_ENTRY_RENAME(Array, operator_arrow_equals, "operator->=")
EHLC_ENTRY(Array, toArray)
EHLC_ENTRY(Array, toTuple)
END_EHLC()

EH_METHOD(Array, initialize) {
	return ehi->to_array(args, context);
}
EH_METHOD(Array, length) {
	ASSERT_NULL_AND_TYPE(array_e, "Array.length");
	return ehretval_t::make_int(obj->get_arrayval()->size());
}
EH_METHOD(Array, operator_arrow) {
	ASSERT_OBJ_TYPE(array_e, "Array.operator->");
	if(args->type() != int_e && args->type() != string_e) {
		throw_TypeError("Invalid type for argument to Array.operator-> (expected String or Integer)", args->type(), ehi);
	}
	eharray_t *arr = obj->get_arrayval();
	if(arr->has(args)) {
		return arr->operator[](args);
	} else {
		return NULL;
	}
}
EH_METHOD(Array, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, array_e, "Array.operator->=");
	ehretval_p index = args->get_tupleval()->get(0);
	if(index->type() != int_e && index->type() != string_e) {
		throw_TypeError("Invalid type for argument to Array.operator->= (expected String or Integer)", index->type(), ehi);
	}
	obj->get_arrayval()->operator[](index) = args->get_tupleval()->get(1);
	return args->get_tupleval()->get(1);
}
EH_METHOD(Array, toArray) {
  ASSERT_NULL_AND_TYPE(array_e, "Array.toArray");
  return obj;
}
EH_METHOD(Array, toTuple) {
  ASSERT_NULL_AND_TYPE(array_e, "Array.toTuple");
  eharray_t *arr = obj->get_arrayval();
  int length = arr->size();
  ehretval_a values(length);
  // We'll say that output order is unspecified
  int i = 0;
  ARRAY_FOR_EACH_INT(arr, member) {
  	values[i++] = member->second;
  }
  ARRAY_FOR_EACH_STRING(arr, member) {
  	values[i++] = member->second;
  }
  return ehi->make_tuple(new ehtuple_t(length, values));
}

START_EHLC(Float)
EHLC_ENTRY(Float, initialize)
EHLC_ENTRY_RENAME(Float, operator_plus, "operator+")
EHLC_ENTRY_RENAME(Float, operator_minus, "operator-")
EHLC_ENTRY_RENAME(Float, operator_times, "operator*")
EHLC_ENTRY_RENAME(Float, operator_divide, "operator/")
EHLC_ENTRY(Float, operator_uminus)
EHLC_ENTRY(Float, compare)
EHLC_ENTRY(Float, abs)
EHLC_ENTRY(Float, toString)
EHLC_ENTRY(Float, toInt)
EHLC_ENTRY(Float, toBool)
EHLC_ENTRY(Float, toFloat)
EHLC_ENTRY(Float, sqrt)
END_EHLC()

EH_METHOD(Float, initialize) {
	return ehi->to_float(args, context);
}
EH_METHOD(Float, operator_plus) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator+");
	ehretval_p operand = ehi->to_float(args, context);
	return ehretval_t::make_float(obj->get_floatval() + operand->get_floatval());
}
EH_METHOD(Float, operator_minus) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator-");
	ehretval_p operand = ehi->to_float(args, context);
	return ehretval_t::make_float(obj->get_floatval() - operand->get_floatval());
}
EH_METHOD(Float, operator_times) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator*");
	ehretval_p operand = ehi->to_float(args, context);
	return ehretval_t::make_float(obj->get_floatval() * operand->get_floatval());
}
EH_METHOD(Float, operator_divide) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator/");
	ehretval_p operand = ehi->to_float(args, context);
	if(operand->get_floatval() == 0.0) {
		throw_MiscellaneousError("Divide by zero in Float.operator/", ehi);
	}
	return ehretval_t::make_float(obj->get_floatval() / operand->get_floatval());
}
EH_METHOD(Float, operator_uminus) {
  ASSERT_NULL_AND_TYPE(float_e, "Float.operator-");
  return ehretval_t::make_float(-obj->get_floatval());
}
EH_METHOD(Float, compare) {
  ASSERT_OBJ_TYPE(float_e, "Float.compare");
  ASSERT_TYPE(args, float_e, "Float.compare");
  float lhs = obj->get_floatval();
  float rhs = args->get_floatval();
  if(lhs < rhs) {
    return ehretval_t::make_int(-1);
  } else if(lhs == rhs) {
    return ehretval_t::make_int(0);
  } else {
    return ehretval_t::make_int(1);
  }
}
EH_METHOD(Float, abs) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.abs");
	return ehretval_t::make_float(abs(obj->get_floatval()));
}
EH_METHOD(Float, toString) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.toString");
	char *buffer = new char[12];
	snprintf(buffer, 12, "%f", obj->get_floatval());
	return ehretval_t::make_string(buffer);
}
EH_METHOD(Float, toInt) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.toInt");
	return ehretval_t::make_int((int) obj->get_floatval());
}
EH_METHOD(Float, toBool) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.toBool");
	return ehretval_t::make_bool(obj->get_floatval() != 0.0);
}
EH_METHOD(Float, toFloat) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.toFloat");
	return obj;
}
EH_METHOD(Float, sqrt) {
  ASSERT_NULL_AND_TYPE(float_e, "Float.sqrt");
  return ehretval_t::make_float(sqrt(obj->get_floatval()));
}

START_EHLC(String)
EHLC_ENTRY(String, initialize)
EHLC_ENTRY_RENAME(String, operator_plus, "operator+")
EHLC_ENTRY_RENAME(String, operator_arrow, "operator->")
EHLC_ENTRY_RENAME(String, operator_arrow_equals, "operator->=")
EHLC_ENTRY(String, compare)
EHLC_ENTRY(String, length)
EHLC_ENTRY(String, toString)
EHLC_ENTRY(String, toInt)
EHLC_ENTRY(String, toFloat)
EHLC_ENTRY(String, toBool)
EHLC_ENTRY(String, toRange)
EHLC_ENTRY(String, charAtPosition)
END_EHLC()

EH_METHOD(String, initialize) {
	return ehi->to_string(args, context);
}
EH_METHOD(String, operator_plus) {
	ASSERT_OBJ_TYPE(string_e, "String.operator+");
	ehretval_p operand = ehi->to_string(args, context);
	ASSERT_TYPE(operand, string_e, "String.operator+");
	size_t len1 = strlen(obj->get_stringval());
	size_t len2 = strlen(operand->get_stringval());
	char *out = new char[len1 + len2 + 1];
	strcpy(out, obj->get_stringval());
	strcpy(out + len1, operand->get_stringval());
	return ehretval_t::make_string(out); 
}
EH_METHOD(String, operator_arrow) {
	ASSERT_OBJ_TYPE(string_e, "String.operator->");
	ASSERT_TYPE(args, int_e, "String.operator->");
	int index = args->get_intval();
	size_t len = strlen(obj->get_stringval());
	// allow negative index
	if(index < 0) {
		index += len;
	}
	if(index < 0 || ((unsigned) index) >= len) {
		throw_ArgumentError_out_of_range("String.operator->", args, ehi);
	}
	char *out = new char[2]();
	out[0] = obj->get_stringval()[index];
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(String, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, string_e, "String.operator->=");
	ehretval_p operand1 = args->get_tupleval()->get(0);
	ASSERT_TYPE(operand1, int_e, "String.operator->=");
	int index = operand1->get_intval();
	size_t len = strlen(obj->get_stringval());
	if(index < 0) {
		index += len;
	}
	if(index < 0 || ((unsigned) index) >= len) {
		throw_ArgumentError_out_of_range("String.operator->=", operand1, ehi);
	}
	ehretval_p operand2 = ehi->to_string(args->get_tupleval()->get(1), context);
	if(strlen(operand2->get_stringval()) == 0) {
		throw_ArgumentError("Argument cannot be a zero-length string", "String.operator->=", args->get_tupleval()->get(1), ehi);
	}
	obj->get_stringval()[index] = operand2->get_stringval()[0];
	return operand2;
}
EH_METHOD(String, compare) {
  ASSERT_OBJ_TYPE(string_e, "String.compare");
  ASSERT_TYPE(args, string_e, "String.compare");
  const char *lhs = obj->get_stringval();
  const char *rhs = args->get_stringval();
  return ehretval_t::make_int(strcmp(lhs, rhs));
}
EH_METHOD(String, length) {
	ASSERT_NULL_AND_TYPE(string_e, "String.length");
	return ehretval_t::make_int(strlen(obj->get_stringval()));
}
EH_METHOD(String, toString) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toString");
	return obj;
}
EH_METHOD(String, toInt) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toInt");
	char *endptr;
	ehretval_p ret = ehretval_t::make_int(strtol(obj->get_stringval(), &endptr, 0));
	// If in == endptr, strtol read no digits and there was no conversion.
	if(obj->get_stringval() == endptr) {
		throw_ArgumentError("Cannot convert String to Integer", "String.toInt", obj, ehi);
	}
	return ret;
}
EH_METHOD(String, toFloat) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toFloat");
	char *endptr;
	ehretval_p ret = ehretval_t::make_float(strtof(obj->get_stringval(), &endptr));
	// If in == endptr, strtof read no digits and there was no conversion.
	if(obj->get_stringval() == endptr) {
		throw_ArgumentError("Cannot convert String to Float", "String.toFloat", obj, ehi);
	}
	return ret;
}
EH_METHOD(String, toBool) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toBool");
	return ehretval_t::make_bool(obj->get_stringval()[0] != '\0');
}
EH_METHOD(String, toRange) {
	ASSERT_NULL_AND_TYPE(string_e, "String.toRange");
	// attempt to find two integers in the string
	int min, max;
	char *in = obj->get_stringval();
	char *ptr;
	// get lower part of range
	for(int i = 0; ; i++) {
		if(in[i] == '\0') {
			throw_ArgumentError("Cannot convert String to Range", "String.toRange", obj, ehi);
		}
		if(isdigit(in[i])) {
			min = strtol(&in[i], &ptr, 0);
			break;
		}
	}
	// get upper bound
	for(int i = 0; ; i++) {
		if(ptr[i] == '\0') {
			throw_ArgumentError("Cannot convert String to Range", "String.toRange", obj, ehi);
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
	ASSERT_OBJ_TYPE(string_e, "String.charAtPosition");
	ASSERT_TYPE(args, int_e, "String.charAtPosition");
	int index = args->get_intval();
	const char *string = obj->get_stringval();
	if(index < 0 || ((unsigned) index) >= strlen(string)) {
		throw_ArgumentError_out_of_range("String.charAtPosition", args, ehi);
	}
	return ehretval_t::make_int(string[index]);
}

START_EHLC(Bool)
EHLC_ENTRY(Bool, initialize)
EHLC_ENTRY(Bool, toString)
EHLC_ENTRY(Bool, toBool)
EHLC_ENTRY(Bool, toInt)
EHLC_ENTRY_RENAME(Bool, operator_bang, "operator!")
END_EHLC()

EH_METHOD(Bool, initialize) {
	return ehi->to_bool(args, context);
}
EH_METHOD(Bool, toString) {
	ASSERT_NULL_AND_TYPE(bool_e, "Bool.toString");
	char *str;
	if(obj->get_boolval()) {
		str = strdup("true");
	} else {
		str = strdup("false");
	}
	return ehretval_t::make_string(str);
}
EH_METHOD(Bool, toBool) {
	ASSERT_NULL_AND_TYPE(bool_e, "Bool.toBool");
	return obj;
}
EH_METHOD(Bool, toInt) {
	ASSERT_NULL_AND_TYPE(bool_e, "Bool.toInt");
	if(obj->get_boolval()) {
		return ehretval_t::make_int(1);
	} else {
		return ehretval_t::make_int(0);
	}
}
EH_METHOD(Bool, operator_bang) {
  ASSERT_NULL_AND_TYPE(bool_e, "Bool.operator!");
  return ehretval_t::make_bool(!obj->get_boolval());
}

START_EHLC(Null)
EHLC_ENTRY(Null, initialize)
EHLC_ENTRY(Null, toString)
EHLC_ENTRY(Null, toBool)
END_EHLC()

EH_METHOD(Null, initialize) {
	return NULL;
}
EH_METHOD(Null, toString) {
	ASSERT_NULL_AND_TYPE(null_e, "Null.toString");
	return ehretval_t::make_string(strdup(""));
}
EH_METHOD(Null, toBool) {
	ASSERT_NULL_AND_TYPE(null_e, "Null.toBool");
	return ehretval_t::make_bool(false);
}

START_EHLC(Range)
EHLC_ENTRY(Range, min)
EHLC_ENTRY(Range, max)
EHLC_ENTRY_RENAME(Range, operator_arrow, "operator->")
EHLC_ENTRY(Range, toString)
EHLC_ENTRY(Range, toArray)
EHLC_ENTRY(Range, toRange)
EHLC_ENTRY(Range, compare)
END_EHLC()

EH_METHOD(Range, initialize) {
	return ehi->to_range(args, context);
}
EH_METHOD(Range, min) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.min");
	return obj->get_rangeval()->min;
}
EH_METHOD(Range, max) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.max");
	return obj->get_rangeval()->max;
}
EH_METHOD(Range, operator_arrow) {
	ASSERT_OBJ_TYPE(range_e, "Range.operator->");
	ASSERT_TYPE(args, int_e, "Range.operator->");
	int index = args->get_intval();
	if(index == 0) {
		return obj->get_rangeval()->min;
	} else if(index == 1) {
		return obj->get_rangeval()->max;
	} else {
		throw_ArgumentError_out_of_range("Range.operator->", args, ehi);
		return NULL;		
	}
}
EH_METHOD(Range, toString) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.toString");
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
	ASSERT_NULL_AND_TYPE(range_e, "Range.toArray");
	ehretval_p array = ehi->make_array(new eharray_t);
	array->get_arrayval()->int_indices[0] = obj->get_rangeval()->min;
	array->get_arrayval()->int_indices[1] = obj->get_rangeval()->max;
	return array;
}
EH_METHOD(Range, toRange) {
	ASSERT_NULL_AND_TYPE(range_e, "Range.toRange");
	return obj;
}
EH_METHOD(Range, compare) {
	ASSERT_OBJ_TYPE(range_e, "Range.compare");
	ASSERT_TYPE(args, range_e, "Range.compare");
	ehretval_p lhs_min = obj->get_rangeval()->min;
	ehretval_p rhs_min = args->get_rangeval()->min;
	int lhs_type = lhs_min->get_full_type();
	int rhs_type = rhs_min->get_full_type();
	if(lhs_type != rhs_type) {
		return ehretval_t::make_int(intcmp(lhs_type, rhs_type));
	}
	// returns -1 if lhs.min < rhs.min and lhs.max < rhs.max or lhs is fully inside rhs (lhs.min > rhs.min, lhs.max < rhs.max),
	// 0 if lhs.min == rhs.min and lhs.max == rhs.max,
	// 1 if lhs.max > rhs.max, lhs.min > rhs.min or lhs.min < rhs.min, lhs.max > rhs.max
	ehretval_p lhs_max = obj->get_rangeval()->max;
	ehretval_p rhs_max = args->get_rangeval()->max;
	ehretval_p min_cmp_p = ehi->call_method(lhs_min, "compare", rhs_min, context);
	if(min_cmp_p->type() != int_e) {
		throw_TypeError("compare must return an Integer", min_cmp_p->type(), ehi);
	}
	int min_cmp = min_cmp_p->get_intval();
	ehretval_p max_cmp_p = ehi->call_method(lhs_max, "compare", rhs_max, context);
	if(max_cmp_p->type() != int_e) {
		throw_TypeError("compare must return an Integer", max_cmp_p->type(), ehi);
	}
	int max_cmp = max_cmp_p->get_intval();
	if(min_cmp == 0) {
		return max_cmp_p;
	} else if(max_cmp == 0) {
		return min_cmp_p;
	} else if((min_cmp < 0 && max_cmp < 0) || (min_cmp > 0 && max_cmp < 0)) {
		return ehretval_t::make_int(-1);
	} else {
		return ehretval_t::make_int(1);
	}
}

START_EHLC(Hash)
EHLC_ENTRY(Hash, toArray)
EHLC_ENTRY_RENAME(Hash, operator_arrow, "operator->")
EHLC_ENTRY_RENAME(Hash, operator_arrow_equals, "operator->=")
EHLC_ENTRY(Hash, has)
END_EHLC()

EH_METHOD(Hash, toArray) {
	ASSERT_NULL_AND_TYPE(hash_e, "Hash.toArray");
	ehhash_t *hash = obj->get_hashval();
	eharray_t *arr = new eharray_t();
	ehretval_p out = ehi->make_array(arr);
	HASH_FOR_EACH(hash, i) {
		arr->string_indices[i->first.c_str()] = i->second;
	}
	return out;
}
EH_METHOD(Hash, operator_arrow) {
	ASSERT_OBJ_TYPE(hash_e, "Hash.operator->");
	ASSERT_TYPE(args, string_e, "Hash.operator->");
	ehhash_t *hash = obj->get_hashval();
	return hash->get(args->get_stringval());
}
EH_METHOD(Hash, operator_arrow_equals) {
	ASSERT_NARGS_AND_TYPE(2, hash_e, "Hash.operator->=");
	ehretval_p index = args->get_tupleval()->get(0);
	ASSERT_TYPE(index, string_e, "Hash.operator->=");
	ehretval_p value = args->get_tupleval()->get(1);
	ehhash_t *hash = obj->get_hashval();
	hash->set(index->get_stringval(), value);
	return value;
}
EH_METHOD(Hash, has) {
	ASSERT_OBJ_TYPE(hash_e, "Hash.has");
	ASSERT_TYPE(args, string_e, "Hash.has");
	ehhash_t *hash = obj->get_hashval();
	return ehretval_t::make_bool(hash->has(args->get_stringval()));
}

START_EHLC(Function)
EHLC_ENTRY_RENAME(Function, operator_colon, "operator:")
EHLC_ENTRY(Function, toString)
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
		throw_TypeError("Invalid base object for Function.operator:", obj->type(), ehi);
	}
	ehfunc_t *f = function_object->get_objectval()->object_data->get_funcval();

	if(f->type == lib_e) {
		return f->libmethod_pointer(object_data, args, parent, ehi);
	}
	ehretval_p newcontext = ehi->object_instantiate(function_object->get_objectval());
	newcontext->get_objectval()->object_data = function_object->get_objectval()->object_data;

	// check parameter count
	if(f->argcount == 0) {
		if(args->type() != null_e) {
			throw_TypeError("Unexpected non-null argument to closure", args->type(), ehi);
		}
	} else if(f->argcount == 1) {
		ehi->set_property(newcontext, f->args[0].name.c_str(), args, newcontext);
	} else {
		if(args->type() != tuple_e) {
			std::ostringstream msg;
			msg << "Argument must be a tuple of size " << f->argcount;
			throw_ArgumentError(msg.str().c_str(), "(closure)", args, ehi);
		} else if(args->get_tupleval()->size() != f->argcount) {
			std::ostringstream msg;
			msg << "Argument must be a tuple of size " << f->argcount;
			throw_ArgumentError(msg.str().c_str(), "(closure)", args, ehi);
		} else {
			// set parameters as necessary
			ehtuple_t *tuple = args->get_tupleval();
			for(int i = 0; i < f->argcount; i++) {
				ehi->set_property(newcontext, f->args[f->argcount - 1 - i].name.c_str(), tuple->get(i), newcontext);
			}			
		}
	}
	// insert self variable with the object_data
	ehmember_p self_member;
	self_member->value = object_data;
	newcontext->get_objectval()->insert("self", self_member);
	
	ehretval_p ret = ehi->eh_execute(f->code, newcontext);
	ehi->returning = false;
	return ret;
}
EH_METHOD(Function, toString) {
	ASSERT_OBJ_TYPE(func_e, "Function.toString");
	ehfunc_t *f = obj->get_funcval();
	if(f->type == lib_e) {
		return ehretval_t::make_string(strdup("func: -> <native code>"));
	} else if(f->argcount == 0) {
		return ehretval_t::make_string(strdup("func: -> <user code>"));
	} else {
		std::ostringstream out;
		out << "func: ";
		for(int i = 0; i < f->argcount; i++) {
			out << f->args[i].name;
			if(i != f->argcount - 1) {
				out << ", ";
			}
		}
		out << " -> <user code>";
		return ehretval_t::make_string(strdup(out.str().c_str()));
	}
}

START_EHLC(Exception)
EHLC_ENTRY(Exception, initialize)
EHLC_ENTRY(Exception, toString)
END_EHLC()

EH_METHOD(Exception, initialize) {
	ASSERT_TYPE(args, string_e, "Exception.initialize");
	return ehretval_t::make_resource(new Exception(strdup(args->get_stringval())));
}
EH_METHOD(Exception, toString) {
	ASSERT_NULL_AND_TYPE(resource_e, "Exception.toString");
	Exception *exc = (Exception *)obj->get_resourceval();
	return ehretval_t::make_string(strdup(exc->msg));
}

START_EHLC(Tuple)
EHLC_ENTRY(Tuple, initialize)
EHLC_ENTRY_RENAME(Tuple, operator_arrow, "operator->")
EHLC_ENTRY(Tuple, length)
EHLC_ENTRY(Tuple, toTuple)
END_EHLC()

EH_METHOD(Tuple, initialize) {
	return ehi->to_tuple(args, context);
}
EH_METHOD(Tuple, operator_arrow) {
	ASSERT_OBJ_TYPE(tuple_e, "Tuple.operator->");
	ASSERT_TYPE(args, int_e, "Tuple.operator->");
	int index = args->get_intval();
	if(index < 0 || index >= obj->get_tupleval()->size()) {
    	throw_ArgumentError_out_of_range("Tuple.operator->", args, ehi);
	}
  	return obj->get_tupleval()->get(index);
}
EH_METHOD(Tuple, length) {
  	ASSERT_NULL_AND_TYPE(tuple_e, "Tuple.length");
  	return ehretval_t::make_int(obj->get_tupleval()->size());
}
EH_METHOD(Tuple, toTuple) {
	ASSERT_OBJ_TYPE(tuple_e, "Tuple.toTuple");
	return obj;
}


START_EHLC(GarbageCollector)
EHLC_ENTRY(GarbageCollector, run)
EHLC_ENTRY(GarbageCollector, stats)
END_EHLC()

EH_METHOD(GarbageCollector, run) {
	ehi->gc.do_collect(ehi->global_object);
	return NULL;
}
EH_METHOD(GarbageCollector, stats) {
	ehi->gc.print_stats();
	return NULL;
}
