#include <cmath>
#include <limits.h>

#include "Integer.hpp"

#include "ArgumentError.hpp"
#include "EmptyIterator.hpp"
#include "MiscellaneousError.hpp"

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
	return ehi->toInteger(args, obj);
}
EH_METHOD(Integer, operator_plus) {
	ASSERT_OBJ_TYPE(Integer, "Integer.operator+");
	if(args->is_a<Float>()) {
		return Float::make((float) obj->get<Integer>() + args->get<Float>());
	} else {
		// always returns an int or throws
		args = ehi->toInteger(args, obj);
		return Integer::make(obj->get<Integer>() + args->get<Integer>());
	}
}
EH_METHOD(Integer, operator_minus) {
	ASSERT_OBJ_TYPE(Integer, "Integer.operator-");
	if(args->is_a<Float>()) {
		return Float::make((float) obj->get<Integer>() - args->get<Float>());
	} else {
		args = ehi->toInteger(args, obj);
		return Integer::make(obj->get<Integer>() - args->get<Integer>());
	}
}
EH_METHOD(Integer, operator_times) {
	ASSERT_OBJ_TYPE(Integer, "Integer.operator*");
	if(args->is_a<Float>()) {
		return Float::make(static_cast<float>(obj->get<Integer>()) * args->get<Float>());
	} else {
		args = ehi->toInteger(args, obj);
		return Integer::make(obj->get<Integer>() * args->get<Integer>());
	}
}
EH_METHOD(Integer, operator_divide) {
	ASSERT_OBJ_TYPE(Integer, "Integer.operator/");
	if(args->is_a<Float>()) {
		float val = args->get<Float>();
		if(val == 0.0) {
			throw_MiscellaneousError("Divide by zero in Integer.operator/", ehi);
		}
		return Float::make(static_cast<float>(obj->get<Integer>()) / val);
	} else {
		args = ehi->toInteger(args, obj);
		int val = args->get<Integer>();
		if(val == 0) {
			throw_MiscellaneousError("Divide by zero in Integer.operator/", ehi);
		}
		return Integer::make(obj->get<Integer>() / args->get<Integer>());
	}
}
EH_METHOD(Integer, operator_modulo) {
	ASSERT_OBJ_TYPE(Integer, "Integer.operator%");
	ehval_p operand = ehi->toInteger(args, obj);
	operand->assert_type<Integer>("Integer.operator%", ehi);
	if(operand->get<Integer>() == 0) {
		throw_MiscellaneousError("Divide by zero in Integer.operator%", ehi);
	}
	return Integer::make(obj->get<Integer>() % operand->get<Integer>());
}
EH_METHOD(Integer, operator_and) {
  ASSERT_OBJ_TYPE(Integer, "Integer.operator&");
  args->assert_type<Integer>("Integer.operator&", ehi);
  return Integer::make(obj->get<Integer>() & args->get<Integer>());
}
EH_METHOD(Integer, operator_or) {
  ASSERT_OBJ_TYPE(Integer, "Integer.operator|");
  args->assert_type<Integer>("Integer.operator|", ehi);
  return Integer::make(obj->get<Integer>() | args->get<Integer>());
}
EH_METHOD(Integer, operator_xor) {
  ASSERT_OBJ_TYPE(Integer, "Integer.operator^");
  args->assert_type<Integer>("Integer.operator^", ehi);
  return Integer::make(obj->get<Integer>() ^ args->get<Integer>());
}
EH_METHOD(Integer, operator_tilde) {
  ASSERT_NULL_AND_TYPE(Integer, "Integer.operator~");
  return Integer::make(~obj->get<Integer>());
}
EH_METHOD(Integer, operator_uminus) {
  ASSERT_NULL_AND_TYPE(Integer, "Integer.operator-");
  return Integer::make(-obj->get<Integer>());
}
EH_METHOD(Integer, compare) {
	ASSERT_OBJ_TYPE(Integer, "Integer.compare");
	args->assert_type<Integer>("Integer.compare", ehi);
	return Integer::make(intcmp(obj->get<Integer>(), args->get<Integer>()));
}
EH_METHOD(Integer, operator_leftshift) {
	ASSERT_OBJ_TYPE(Integer, "Integer.operator<<");
	args->assert_type<Integer>("Integer.operator<<", ehi);
	return Integer::make(obj->get<Integer>() << args->get<Integer>());
}
EH_METHOD(Integer, operator_rightshift) {
	ASSERT_OBJ_TYPE(Integer, "Integer.operator>>");
	args->assert_type<Integer>("Integer.operator>>", ehi);
	return Integer::make(obj->get<Integer>() >> args->get<Integer>());
}
EH_METHOD(Integer, abs) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.abs");
	return Integer::make(abs(obj->get<Integer>()));
}
EH_METHOD(Integer, getBit) {
	ASSERT_OBJ_TYPE(Integer, "Integer.getBit");
	args->assert_type<Integer>("Integer.getBit", ehi);
	int index = args->get<Integer>();
	if(index < 0 || ((unsigned) index) >= sizeof(int) * 8) {
		throw_ArgumentError_out_of_range("Integer.getBit", args, ehi);
	}
	// get mask
	const unsigned int mask = (1 << (sizeof(int) * 8 - 1)) >> index;
	// apply mask
	return Bool::make((obj->get<Integer>() & mask) >> (sizeof(int) * 8 - 1 - mask));
}
EH_METHOD(Integer, setBit) {
	ASSERT_NARGS_AND_TYPE(2, Integer, "Integer.setBit");
	ehval_p operand = args->get<Tuple>()->get(0);
	operand->assert_type<Integer>("Integer.setBit", ehi);
	int index = operand->get<Integer>();
	if(index < 0 || ((unsigned int) index) >= sizeof(int) * 8) {
		throw_ArgumentError_out_of_range("Integer.setBit", operand, ehi);
	}
	int new_value = 0;
	ehval_p value = args->get<Tuple>()->get(1);
	if(value->is_a<Integer>()) {
		int int_value = value->get<Integer>();
		if(int_value != 0 && int_value != 1) {
			throw_ArgumentError_out_of_range("Integer.setBit", value, ehi);
		}
		new_value = int_value;
	} else if(value->is_a<Bool>()) {
		new_value = value->get<Bool>();
	} else {
		throw_TypeError("Second argument to Integer.setBit must be an Integer or Bool", value, ehi);
	}
	const unsigned int mask = (1u << (sizeof(int) * 8 - 1)) >> index;
	int out = obj->get<Integer>();
	if(new_value) {
		out |= mask;
	} else {
		out &= ~mask;
	}
	return Integer::make(out);
}
EH_METHOD(Integer, length) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.length");
	return Integer::make(sizeof(int));
}
EH_METHOD(Integer, toString) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.toString");
	// INT_MAX has 10 decimal digits on this computer, so 12 (including sign and
	// null terminator) should suffice for the result string
	char *buffer = new char[12]();
	snprintf(buffer, 12, "%d", obj->get<Integer>());
	return String::make(buffer);
}
EH_METHOD(Integer, toBool) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.toBool");
	return Bool::make(obj->get<Integer>() != 0);
}
EH_METHOD(Integer, toFloat) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.toFloat");
	return Float::make((float) obj->get<Integer>());
}
EH_METHOD(Integer, toInt) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.toInt");
	return obj;
}
EH_METHOD(Integer, toChar) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.toChar");
	if(obj->get<Integer>() < 0 || obj->get<Integer>() > SCHAR_MAX) {
		throw_ArgumentError_out_of_range("Integer.toChar", obj, ehi);
	}
	char *out = new char[2];
	out[0] = static_cast<char>(obj->get<Integer>());
	out[1] = '\0';
	return String::make(out);
}
EH_METHOD(Integer, sqrt) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.sqrt");
	return Integer::make((int) sqrt((double) obj->get<Integer>()));
}
EH_METHOD(Integer, getIterator) {
	ASSERT_NULL_AND_TYPE(Integer, "Integer.getIterator");
	ehval_p class_obj = ehi->get_parent()->repo.get_object(obj);
	ehval_p iterator = class_obj->get_property("Iterator", obj, ehi);
	return ehi->call_method(iterator, "new", obj, obj);
}

bool Integer_Iterator::t::has_next() {
	return this->current < this->max;
}
int Integer_Iterator::t::next() {
	assert(has_next());
	return this->current++;
}
EH_METHOD(Integer_Iterator, initialize) {
	args->assert_type<Integer>("Integer,Iterator.initialize", ehi);
	return Integer_Iterator::make(args->get<Integer>());
}
EH_METHOD(Integer_Iterator, hasNext) {
	args->assert_type<Null>("Integer.Iterator.hasNext", ehi);
	ASSERT_RESOURCE(Integer_Iterator, "Integer.Iterator.hasNext");
	return Bool::make(data->has_next());
}
EH_METHOD(Integer_Iterator, next) {
	args->assert_type<Null>("Integer.Iterator.next", ehi);
	ASSERT_RESOURCE(Integer_Iterator, "Integer.Iterator.next");
	if(!data->has_next()) {
		throw_EmptyIterator(ehi);
	}
	return Integer::make(data->next());
}
EH_INITIALIZER(Integer_Iterator) {
	REGISTER_METHOD(Integer_Iterator, initialize);
	REGISTER_METHOD(Integer_Iterator, hasNext);
	REGISTER_METHOD(Integer_Iterator, next);
}
