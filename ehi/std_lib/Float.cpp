/*
 * Float
 * Represents floating-point numbers. Most Float methods that take arguments
 * (e.g., Float.operator+) cast their arguments to Float (by calling toFloat)
 * before making calculations.
 */
#include <cmath>

#include "ByteArray.hpp"
#include "Float.hpp"
#include "MiscellaneousError.hpp"

EH_INITIALIZER(Float) {
	REGISTER_CONSTRUCTOR(Float);
	REGISTER_METHOD_RENAME(Float, operator_plus, "operator+");
	REGISTER_METHOD_RENAME(Float, operator_minus, "operator-");
	REGISTER_METHOD_RENAME(Float, operator_times, "operator*");
	REGISTER_METHOD_RENAME(Float, operator_divide, "operator/");
	REGISTER_METHOD(Float, compare);
	REGISTER_METHOD(Float, abs);
	REGISTER_METHOD(Float, toString);
	REGISTER_METHOD(Float, toInteger);
	REGISTER_METHOD(Float, toBool);
	REGISTER_METHOD(Float, toFloat);
	REGISTER_METHOD(Float, sqrt);
	REGISTER_METHOD(Float, round);
	REGISTER_METHOD(Float, toBytes);
}

/*
 * @description Initializer. Calls toFloat on its argument.
 * @argument Object to cast to Float
 * @returns N/A
 */
EH_METHOD(Float, operator_colon) {
	return ehi->toFloat(args, obj);
}

/*
 * @description Sums two floats.
 * @argument Object to add
 * @returns Sum
 */
EH_METHOD(Float, operator_plus) {
	ASSERT_OBJ_TYPE(Float, "Float.operator+");
	ehval_p operand = ehi->toFloat(args, obj);
	return Float::make(obj->get<Float>() + operand->get<Float>());
}

/*
 * @description Subtracts one float from another
 * @argument Object to subtract
 * @returns Difference
 */
EH_METHOD(Float, operator_minus) {
	ASSERT_OBJ_TYPE(Float, "Float.operator-");
	ehval_p operand = ehi->toFloat(args, obj);
	return Float::make(obj->get<Float>() - operand->get<Float>());
}

/*
 * @description Multiply one float by another.
 * @argument Object to multiply by
 * @returns Product
 */
EH_METHOD(Float, operator_times) {
	ASSERT_OBJ_TYPE(Float, "Float.operator*");
	ehval_p operand = ehi->toFloat(args, obj);
	return Float::make(obj->get<Float>() * operand->get<Float>());
}

/*
 * @description Divide one float by another. Throws a MiscellaneousError on
 * division by zero.
 * @argument Object to divide by
 * @returns Quotient
 */
EH_METHOD(Float, operator_divide) {
	ASSERT_OBJ_TYPE(Float, "Float.operator/");
	ehval_p operand = ehi->toFloat(args, obj);
	if(operand->get<Float>() == 0.0) {
		throw_MiscellaneousError("Divide by zero in Float.operator/", ehi);
	}
	return Float::make(obj->get<Float>() / operand->get<Float>());
}

/*
 * @description Compare two floats
 * @argument Float to compare to
 * @returns Integer (as specified for Object.compare)
 */
EH_METHOD(Float, compare) {
	ASSERT_OBJ_TYPE(Float, "Float.compare");
	args->assert_type<Float>("Float.compare", ehi);
	float lhs = obj->get<Float>();
	float rhs = args->get<Float>();
	if(lhs < rhs) {
		return Integer::make(-1);
	} else if(lhs == rhs) {
		return Integer::make(0);
	} else {
		return Integer::make(1);
	}
}

/*
 * @description Returns the absolute value of a float.
 * @argument None
 * @returns Absolute value
 */
EH_METHOD(Float, abs) {
	ASSERT_NULL_AND_TYPE(Float, "Float.abs");
	return Float::make(static_cast<Float::type>(abs(obj->get<Float>())));
}

/*
 * @description Produce a string representation of the float
 * @argument None
 * @returns String
 */
EH_METHOD(Float, toString) {
	ASSERT_NULL_AND_TYPE(Float, "Float.toString");
	char *buffer = new char[12];
	snprintf(buffer, 12, "%f", obj->get<Float>());
	return String::make(buffer);
}

/*
 * @description Truncates the float to an int
 * @argument None
 * @returns Integer
 */
EH_METHOD(Float, toInteger) {
	ASSERT_NULL_AND_TYPE(Float, "Float.toInteger");
	return Integer::make((int) obj->get<Float>());
}

/*
 * @description Returns false for 0.0 and true for anything else
 * @argument None
 * @returns Bool
 */
EH_METHOD(Float, toBool) {
	ASSERT_NULL_AND_TYPE(Float, "Float.toBool");
	return Bool::make(obj->get<Float>() != 0.0);
}

/*
 * @description Does nothing.
 * @argument None
 * @returns The float itself
 */
EH_METHOD(Float, toFloat) {
	ASSERT_NULL_AND_TYPE(Float, "Float.toFloat");
	return obj;
}

/*
 * @description Takes the square root of a float.
 * @argument None
 * @returns Square root
 */
EH_METHOD(Float, sqrt) {
  ASSERT_NULL_AND_TYPE(Float, "Float.sqrt");
  return Float::make(static_cast<Float::type>(sqrt(obj->get<Float>())));
}

/*
 * @description Rounds to the nearest integer.
 * @argument None
 * @returns Integer
 */
EH_METHOD(Float, round) {
	ASSERT_NULL_AND_TYPE(Float, "Float.round");
	return Integer::make(lround(obj->get<Float>()));
}

/*
 * @description Returns a byte representation of a float. Useful only for
 * applications representing floats within a binary stream.
 * @argument None
 * @returns ByteArray
 */
EH_METHOD(Float, toBytes) {
	ASSERT_NULL_AND_TYPE(Float, "Float.round");
	// TODO: change this when we switch the internal representation of EH Floats to a double
	auto float_size = sizeof(float);
	ehval_p ba_obj = ByteArray::make(float_size);
	auto ba = ba_obj->get<ByteArray>();
	auto input = obj->get<Float>();
	memcpy(ba->content, &input, float_size);
	return ba_obj;
}

