/*
 * Float
 * Represents floating-point numbers. Most Float methods that take arguments
 * (e.g., Float.operator+) cast their arguments to Float (by calling toFloat)
 * before making calculations.
 */
#include <cmath>

#include "Float.hpp"
#include "MiscellaneousError.hpp"

EH_INITIALIZER(Float) {
	REGISTER_METHOD(Float, initialize);
	REGISTER_METHOD_RENAME(Float, operator_plus, "operator+");
	REGISTER_METHOD_RENAME(Float, operator_minus, "operator-");
	REGISTER_METHOD_RENAME(Float, operator_times, "operator*");
	REGISTER_METHOD_RENAME(Float, operator_divide, "operator/");
	REGISTER_METHOD(Float, compare);
	REGISTER_METHOD(Float, abs);
	REGISTER_METHOD(Float, toString);
	REGISTER_METHOD(Float, toInt);
	REGISTER_METHOD(Float, toBool);
	REGISTER_METHOD(Float, toFloat);
	REGISTER_METHOD(Float, sqrt);
}

/*
 * @description Initializer. Calls toFloat on its argument.
 * @argument Object to cast to Float
 * @returns N/A
 */
EH_METHOD(Float, initialize) {
	return ehi->to_float(args, obj);
}

/*
 * @description Sums two floats.
 * @argument Object to add
 * @returns Sum
 */
EH_METHOD(Float, operator_plus) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator+");
	ehretval_p operand = ehi->to_float(args, obj);
	return ehretval_t::make_float(obj->get_floatval() + operand->get_floatval());
}

/*
 * @description Subtracts one float from another
 * @argument Object to subtract
 * @returns Difference
 */
EH_METHOD(Float, operator_minus) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator-");
	ehretval_p operand = ehi->to_float(args, obj);
	return ehretval_t::make_float(obj->get_floatval() - operand->get_floatval());
}

/*
 * @description Multiply one float by another.
 * @argument Object to multiply by
 * @returns Product
 */
EH_METHOD(Float, operator_times) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator*");
	ehretval_p operand = ehi->to_float(args, obj);
	return ehretval_t::make_float(obj->get_floatval() * operand->get_floatval());
}

/*
 * @description Divide one float by another. Throws a MiscellaneousError on
 * division by zero.
 * @argument Object to divide by
 * @returns Quotient
 */
EH_METHOD(Float, operator_divide) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator/");
	ehretval_p operand = ehi->to_float(args, obj);
	if(operand->get_floatval() == 0.0) {
		throw_MiscellaneousError("Divide by zero in Float.operator/", ehi);
	}
	return ehretval_t::make_float(obj->get_floatval() / operand->get_floatval());
}

/*
 * @description Compare two floats
 * @argument Float to compare to
 * @returns Integer (as specified for Object.compare)
 */
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

/*
 * @description Returns the absolute value of a float.
 * @argument None
 * @returns Absolute value
 */
EH_METHOD(Float, abs) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.abs");
	return ehretval_t::make_float(abs(obj->get_floatval()));
}

/*
 * @description Produce a string representation of the float
 * @argument None
 * @returns String
 */
EH_METHOD(Float, toString) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.toString");
	char *buffer = new char[12];
	snprintf(buffer, 12, "%f", obj->get_floatval());
	return ehretval_t::make_string(buffer);
}

/*
 * @description Truncates the float to an int
 * @argument None
 * @returns Integer
 */
EH_METHOD(Float, toInt) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.toInt");
	return ehretval_t::make_int((int) obj->get_floatval());
}

/*
 * @description Returns false for 0.0 and true for anything else
 * @argument None
 * @returns Bool
 */
EH_METHOD(Float, toBool) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.toBool");
	return ehretval_t::make_bool(obj->get_floatval() != 0.0);
}

/*
 * @description Does nothing.
 * @argument None
 * @returns The float itself
 */
EH_METHOD(Float, toFloat) {
	ASSERT_NULL_AND_TYPE(float_e, "Float.toFloat");
	return obj;
}

/*
 * @description Takes the square root of a float.
 * @argument None
 * @returns Square root
 */
EH_METHOD(Float, sqrt) {
  ASSERT_NULL_AND_TYPE(float_e, "Float.sqrt");
  return ehretval_t::make_float(sqrt(obj->get_floatval()));
}
