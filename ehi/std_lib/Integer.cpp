#include <cmath>

#include "Integer.h"
#include "MiscellaneousError.h"

START_EHLC(Integer)
EHLC_ENTRY(Integer, initialize)
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
	return ehi->to_int(args, obj);
}
EH_METHOD(Integer, operator_plus) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator+");
	if(args->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() + args->get_floatval());
	} else {
		// always returns an int or throws
		args = ehi->to_int(args, obj);
		return ehretval_t::make_int(obj->get_intval() + args->get_intval());
	}
}
EH_METHOD(Integer, operator_minus) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator-");
	if(args->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() - args->get_floatval());
	} else {
		args = ehi->to_int(args, obj);
		return ehretval_t::make_int(obj->get_intval() - args->get_intval());
	}
}
EH_METHOD(Integer, operator_times) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator*");
	if(args->type() == float_e) {
		return ehretval_t::make_float((float) obj->get_intval() * args->get_floatval());
	} else {
		args = ehi->to_int(args, obj);
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
		args = ehi->to_int(args, obj);
		int val = args->get_intval();
		if(val == 0) {
			throw_MiscellaneousError("Divide by zero in Integer.operator/", ehi);
		}
		return ehretval_t::make_int(obj->get_intval() / args->get_intval());
	}
}
EH_METHOD(Integer, operator_modulo) {
	ASSERT_OBJ_TYPE(int_e, "Integer.operator%");
	ehretval_p operand = ehi->to_int(args, obj);
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
