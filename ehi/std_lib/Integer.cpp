#include <cmath>
#include <limits.h>

#include "Integer.h"
#include "MiscellaneousError.h"

EH_INITIALIZER(Integer) {
	REGISTER_METHOD(Integer, initialize);
	REGISTER_METHOD_RENAME(Integer, operator_plus, "operator+");
	REGISTER_METHOD_RENAME(Integer, operator_minus, "operator-");
	REGISTER_METHOD_RENAME(Integer, operator_times, "operator*");
	REGISTER_METHOD_RENAME(Integer, operator_divide, "operator/");
	REGISTER_METHOD_RENAME(Integer, operator_modulo, "operator%");
	REGISTER_METHOD_RENAME(Integer, operator_and, "operator&");
	REGISTER_METHOD_RENAME(Integer, operator_or, "operator|");
	REGISTER_METHOD_RENAME(Integer, operator_xor, "operator^");
	REGISTER_METHOD_RENAME(Integer, operator_tilde, "operator~");
	REGISTER_METHOD_RENAME(Integer, operator_leftshift, "operator<<");
	REGISTER_METHOD_RENAME(Integer, operator_rightshift, "operator>>");
	REGISTER_METHOD(Integer, operator_uminus);
	REGISTER_METHOD(Integer, compare);
	REGISTER_METHOD(Integer, abs);
	REGISTER_METHOD(Integer, getBit);
	REGISTER_METHOD(Integer, setBit);
	REGISTER_METHOD(Integer, length);
	REGISTER_METHOD(Integer, toString);
	REGISTER_METHOD(Integer, toBool);
	REGISTER_METHOD(Integer, toFloat);
	REGISTER_METHOD(Integer, toInt);
	REGISTER_METHOD(Integer, toChar);
	REGISTER_METHOD(Integer, sqrt);
	REGISTER_METHOD(Integer, getIterator);
	REGISTER_CLASS(Integer, Iterator);
}

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
EH_METHOD(Integer, toChar) {
	ASSERT_NULL_AND_TYPE(int_e, "Integer.toChar");
	if(obj->get_intval() < 0 || obj->get_intval() > SCHAR_MAX) {
		throw_ArgumentError_out_of_range("Integer.toChar", obj, ehi);
	}
	char *out = new char[2];
	out[0] = obj->get_intval();
	out[1] = '\0';
	return ehretval_t::make_string(out);
}
EH_METHOD(Integer, sqrt) {
	ASSERT_NULL_AND_TYPE(int_e, "Integer.sqrt");
	return ehretval_t::make_int((int) sqrt((double) obj->get_intval()));
}
EH_METHOD(Integer, getIterator) {
	ASSERT_NULL_AND_TYPE(int_e, "Integer.getIterator");
	ehretval_p iterator = ehi->get_property(obj, "Iterator", obj);
	return ehi->call_method(iterator, "new", obj, obj);
}

bool Integer_Iterator::has_next() {
	return this->current < this->max;
}
int Integer_Iterator::next() {
	assert(has_next());
	return this->current++;
}
EH_METHOD(Integer_Iterator, initialize) {
	ASSERT_TYPE(args, int_e, "Integer.Iterator.initialize");
	Integer_Iterator *data = new Integer_Iterator(args->get_intval());
	return ehretval_t::make_resource(data);
}
EH_METHOD(Integer_Iterator, hasNext) {
	ASSERT_TYPE(args, null_e, "Integer.Iterator.hasNext");
	Integer_Iterator *data = (Integer_Iterator *)obj->get_resourceval();
	return ehretval_t::make_bool(data->has_next());
}
EH_METHOD(Integer_Iterator, next) {
	ASSERT_TYPE(args, null_e, "Integer.Iterator.next");
	Integer_Iterator *data = (Integer_Iterator *)obj->get_resourceval();
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return ehretval_t::make_int(data->next());
}
EH_INITIALIZER(Integer_Iterator) {
	REGISTER_METHOD(Integer_Iterator, initialize);
	REGISTER_METHOD(Integer_Iterator, hasNext);
	REGISTER_METHOD(Integer_Iterator, next);
}
