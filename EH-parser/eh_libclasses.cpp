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
  eh_error_type("argument to" #method, operand->type(), enotice_e); \
  return NULL; \
}

EH_METHOD(Object, toString) {
  ASSERT_NARGS(0, "Object.toString");
  const size_t len = sizeof(void *) * 2 + 3;
  char *out = new char[len]();
  snprintf(out, len, "0x%x", reinterpret_cast<unsigned long>(obj.operator->()));
  return ehretval_t::make_string(out);
}

START_EHLC(CountClass)
EHLC_ENTRY(CountClass, docount)
EHLC_ENTRY(CountClass, setcount)
END_EHLC()

EH_METHOD(CountClass, docount) {
	if(nargs != 0) {
		eh_error_argcount_lib("CountClass::docount", 0, nargs);
		return NULL;
	}
	CountClass *selfptr = (CountClass *)obj->get_resourceval();
	return ehretval_t::make_int(++selfptr->count);
}
EH_METHOD(CountClass, setcount) {
	if(nargs != 1) {
		eh_error_argcount_lib("CountClass::setcount", 1, nargs);
		return NULL;
	}
	CountClass *selfptr = (CountClass *)obj->get_resourceval();

	ehretval_p newcounter = eh_xtoint(args[0]);
	if(newcounter->type() != int_e) {
		return NULL;
	}

	selfptr->count = newcounter->get_intval();
	return ehretval_t::make_bool(true);
}

START_EHLC(File)
EHLC_ENTRY(File, open)
EHLC_ENTRY(File, getc)
EHLC_ENTRY(File, gets)
EHLC_ENTRY(File, puts)
EHLC_ENTRY(File, close)
END_EHLC()

EH_METHOD(File, open) {
	File *selfptr = (File *) obj->get_resourceval();

	if(nargs != 1) {
		eh_error_argcount_lib("File.open", 1, nargs);
		return NULL;
	}
	ehretval_p filename = eh_xtostring(args[0]);
	if(filename->type() != string_e) {
		return NULL;
	}
	FILE *mfile = fopen(filename->get_stringval(), "r+");
	if(mfile == NULL) {
		return NULL;
	}
	selfptr->descriptor = mfile;
	return ehretval_t::make_bool(true);
}

EH_METHOD(File, getc) {
	File *selfptr = (File *) obj->get_resourceval();
	if(nargs != 0) {
		eh_error_argcount_lib("File::getc", 0, nargs);
		return NULL;
	}

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
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	if(nargs != 0) {
		eh_error_argcount_lib("File::gets", 0, nargs);
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
	
	if(nargs != 1) {
		eh_error_argcount_lib("File::puts", 1, nargs);
		return NULL;
	}
	ehretval_p str = eh_xtostring(args[0]);
	if(str->type() != string_e) {
		return NULL;
	}

	int count = fputs(str->get_stringval(), selfptr->descriptor);
	
	if(count == EOF) {
		return ehretval_t::make_bool(false);
	} else {
		return ehretval_t::make_bool(true);
	}
}

EH_METHOD(File, close) {
	File *selfptr = (File *) obj->get_resourceval();
	if(selfptr->descriptor == NULL) {
		return NULL;
	}
	if(nargs != 0) {
		eh_error_argcount_lib("File::close", 0, nargs);
		return NULL;
	}

	fclose(selfptr->descriptor);
	selfptr->descriptor = NULL;
	return NULL;
}

START_EHLC(Integer)
EHLC_ENTRY(Integer, operator_plus)
EHLC_ENTRY(Integer, abs)
END_EHLC()

EH_METHOD(Integer, operator_plus) {
  ASSERT_NARGS(1, "Integer.operator+");
  ehretval_p operand = args[0];
  if(operand->type() == float_e) {
    return ehretval_t::make_float((float) obj->get_intval() + operand->get_floatval());
  } else {
    operand = eh_xtoint(operand);
    if(operand->type() == int_e) {
      return ehretval_t::make_int(obj->get_intval() + operand->get_intval());
    } else {
      eh_error_type("argument to Integer.operator+", args[0]->type(), enotice_e);
      return NULL;
    }
  }
}
EH_METHOD(Integer, abs) {
  ASSERT_NARGS(0, "Integer.abs");
  return ehretval_t::make_int(abs(obj->get_intval()));
}
EH_METHOD(Integer, getBit) {
  ASSERT_NARGS(1, "Integer.getBit");
  ehretval_p operand = args[0];
  ASSERT_TYPE(operand, int_e, "Integer.getBit");
  int index = operand->get_intval();
  if(index < 0 || index >= sizeof(int)) {
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
  ASSERT_NARGS(2, "Integer.setBit");
  ehretval_p operand = args[0];
  ASSERT_TYPE(operand, int_e, "Integer.setBit");
  int index = operand->get_intval();
  if(index < 0 || index >= sizeof(int)) {
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
  ASSERT_NARGS(0, "Integer.length");
  return ehretval_t::make_int(sizeof(int));
}
EH_METHOD(Integer, toString) {
  ASSERT_NARGS(0, "Integer.toString");
	// INT_MAX has 10 decimal digits on this computer, so 12 (including sign and
	// null terminator) should suffice for the result string
	char *buffer = new char[12]();
	snprintf(buffer, 12, "%d", obj->get_intval());
	return ehretval_t::make_string(buffer);
}
EH_METHOD(Integer, toBool) {
  ASSERT_NARGS(0, "Integer.toBool");
  return ehretval_t::make_bool(obj->get_intval() != 0);
}
EH_METHOD(Integer, toFloat) {
  ASSERT_NARGS(0, "Integer.toFloat");
  return ehretval_t::make_float((float) obj->get_floatval());
}

START_EHLC(Array)
EHLC_ENTRY(Array, length)
EHLC_ENTRY(Array, operator_arrow)
EHLC_ENTRY(Array, operator_arrow_equals)
END_EHLC()

EH_METHOD(Array, length) {
  ASSERT_NARGS(0, "Array.length");
  return ehretval_t::make_int(obj->get_arrayval()->size());
}
EH_METHOD(Array, operator_arrow) {
  ASSERT_NARGS(1, "Array.operator->");
  try {
    return obj->get_arrayval()->operator[](args[0]);
  } catch(unknown_value_exception &e) {
    return NULL;
  }
}
EH_METHOD(Array, operator_arrow_equals) {
  ASSERT_NARGS(2, "Array.operator->=");
  try {
    obj->get_arrayval()->operator[](args[0]) = args[1];
    return args[1];
  } catch(unknown_value_exception &e) {
    return NULL;
  }
}

START_EHLC(Float)
EHLC_ENTRY(Float, operator_plus)
EHLC_ENTRY(Float, abs)
END_EHLC()

EH_METHOD(Float, operator_plus) {
  ASSERT_NARGS(1, "Float.operator+");
  ehretval_p operand = eh_xtofloat(args[0]);
  if(operand->type() == float_e) {
    return ehretval_t::make_float(obj->get_floatval() + operand->get_floatval());
  } else {
    eh_error_type("argument to Float.operator+", args[0]->type(), enotice_e);
    return NULL;
  }
}
EH_METHOD(Float, abs) {
  ASSERT_NARGS(0, "Float.abs");
  return ehretval_t::make_float(abs(obj->get_floatval()));
}
EH_METHOD(Float, toString) {
  ASSERT_NARGS(0, "Float.toString");
	char *buffer = new char[12];
	snprintf(buffer, 12, "%f", obj->get_floatval());
	return ehretval_t::make_string(buffer);
}
EH_METHOD(Float, toInt) {
  ASSERT_NARGS(0, "Float.toInt");
  return ehretval_t::make_int((int) obj->get_floatval());
}
EH_METHOD(Float, toBool) {
  ASSERT_NARGS(0, "Float.toBool");
  return ehretval_t::make_bool(obj->get_floatval() != 0.0);
}

START_EHLC(String)
EHLC_ENTRY(String, operator_plus)
EHLC_ENTRY(String, operator_arrow)
EHLC_ENTRY(String, operator_arrow_equals)
EHLC_ENTRY(String, length)
END_EHLC()

EH_METHOD(String, operator_plus) {
  ASSERT_NARGS(1, "String.operator+");
  ehretval_p operand = eh_xtostring(args[0]);
  ASSERT_TYPE(operand, string_e, "String.operator+");
  size_t len1 = strlen(obj->get_stringval());
  size_t len2 = strlen(operand->get_stringval());
  char *out = new char[len1 + len2 + 1];
  strcpy(out, obj->get_stringval());
  strcpy(out + len1, operand->get_stringval());
  return ehretval_t::make_string(out); 
}
EH_METHOD(String, operator_arrow) {
  ASSERT_NARGS(1, "String.operator->");
  ehretval_p operand = args[0];
  ASSERT_TYPE(operand, int_e, "String.operator->");
  int index = operand->get_intval();
  size_t len = strlen(obj->get_stringval());
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
  ASSERT_NARGS(2, "String.operator->=");
  ehretval_p operand1 = args[0];
  ASSERT_TYPE(operand1, int_e, "String.operator->");
  int index = operand1->get_intval();
  size_t len = strlen(obj->get_stringval());
  if(index < 0 || index >= len) {
    eh_error_invalid_argument("String.operator->", 1);
    return NULL;
  }
  ehretval_p operand2 = eh_xtostring(args[1]);
  ASSERT_TYPE(operand2, string_e, "String.operator->");
  if(strlen(operand2->get_stringval()) == 0) {
    eh_error_invalid_argument("String.operator->=", 2);
    return NULL;
  }
  obj->get_stringval()[index] = operand2->get_stringval()[0];
  return operand2;
}
EH_METHOD(String, length) {
  ASSERT_NARGS(0, "String.length");
  return ehretval_t::make_int(strlen(obj->get_stringval()));
}
EH_METHOD(String, toString) {
  ASSERT_NARGS(0, "String.toString");
  return obj;
}
EH_METHOD(String, toInt) {
  ASSERT_NARGS(0, "String.toInt");
	char *endptr;
	ehretval_p ret = ehretval_t::make_int(strtol(obj->get_stringval(), &endptr, 0));
	// If in == endptr, strtol read no digits and there was no conversion.
	if(obj->get_stringval() == endptr) {
	  // get very angry
	}
	return ret;
  
}
EH_METHOD(String, toFloat) {
  ASSERT_NARGS(0, "String.toFloat");
	char *endptr;
	ehretval_p ret = ehretval_t::make_float(strtof(obj->get_stringval(), &endptr));
	// If in == endptr, strtof read no digits and there was no conversion.
	if(obj->get_stringval() == endptr) {
	  // get very angry
	}
	return ret;
}
EH_METHOD(String, toBool) {
  ASSERT_NARGS(0, "String.toBool");
  return ehretval_t::make_bool(obj->get_stringval()[0] != '\0');
}
EH_METHOD(String, toRange) {
  ASSERT_NARGS(0, "String.toRange");
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
	return ehretval_t::make_range(range);
}

EH_METHOD(Bool, toString) {
  ASSERT_NARGS(0, "Bool.toString");
  char *str;
  if(obj->get_boolval()) {
    str = strdup("true");
  } else {
    str = strdup("false");
  }
  return ehretval_t::make_string(str);
}
EH_METHOD(Bool, toBool) {
  ASSERT_NARGS(0, "Bool.toBool");
  return obj;
}

EH_METHOD(Null, toString) {
  ASSERT_NARGS(0, "Null.toString");
  return ehretval_t::make_string(strdup(""));
}
EH_METHOD(Null, toBool) {
  ASSERT_NARGS(0, "Null.toBool");
  return ehretval_t::make_bool(false);
}

EH_METHOD(Range, min) {
  ASSERT_NARGS(0, "Range.min");
  return obj->get_rangeval()->min;
}
EH_METHOD(Range, max) {
  ASSERT_NARGS(0, "Range.max");
  return obj->get_rangeval()->max;
}
EH_METHOD(Range, operator_arrow) {
  ASSERT_NARGS(1, "Range.operator->");
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
  ASSERT_NARGS(0, "Range.toString");
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
  ASSERT_NARGS(0, "Range.toArray");
  ehretval_p array = ehi->make_array(new eharray_t);
  array->get_arrayval()->int_indices[0] = obj->get_rangeval()->min;
  array->get_arrayval()->int_indices[1] = obj->get_rangeval()->max;
  return array;
}
