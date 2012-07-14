/*
 * eh_libclasses.cpp
 *
 * EH library classes
 */
#include "eh_libclasses.h"
#include "eh_error.h"
#include <cmath>

#define ASSERT_NARGS(count, method) if(nargs != count) { \
  eh_error_argcount_lib(#method, count, nargs); \
  return NULL; \
}
#define ASSERT_TYPE(operand, ehtype, method) if(operand->type() != ehtype) { \
  eh_error_type("argument to" #method, operand->type(), enotice_e); \
  return NULL; \
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

START_EHLC(Array)
EHLC_ENTRY(Array, count)
EHLC_ENTRY(Array, operator_arrow)
EHLC_ENTRY(Array, operator_arrow_equals)
END_EHLC()

EH_METHOD(Array, count) {
  ASSERT_NARGS(0, "Array.count");
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
    eh_error_invalid_argument("String.operator->", 1);
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
