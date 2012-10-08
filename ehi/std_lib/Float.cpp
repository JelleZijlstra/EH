#include <cmath>

#include "Float.h"
#include "MiscellaneousError.h"

EH_INITIALIZER(Float) {
	REGISTER_METHOD(Float, initialize);
	REGISTER_METHOD_RENAME(Float, operator_plus, "operator+");
	REGISTER_METHOD_RENAME(Float, operator_minus, "operator-");
	REGISTER_METHOD_RENAME(Float, operator_times, "operator*");
	REGISTER_METHOD_RENAME(Float, operator_divide, "operator/");
	REGISTER_METHOD(Float, operator_uminus);
	REGISTER_METHOD(Float, compare);
	REGISTER_METHOD(Float, abs);
	REGISTER_METHOD(Float, toString);
	REGISTER_METHOD(Float, toInt);
	REGISTER_METHOD(Float, toBool);
	REGISTER_METHOD(Float, toFloat);
	REGISTER_METHOD(Float, sqrt);
}

EH_METHOD(Float, initialize) {
	return ehi->to_float(args, obj);
}
EH_METHOD(Float, operator_plus) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator+");
	ehretval_p operand = ehi->to_float(args, obj);
	return ehretval_t::make_float(obj->get_floatval() + operand->get_floatval());
}
EH_METHOD(Float, operator_minus) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator-");
	ehretval_p operand = ehi->to_float(args, obj);
	return ehretval_t::make_float(obj->get_floatval() - operand->get_floatval());
}
EH_METHOD(Float, operator_times) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator*");
	ehretval_p operand = ehi->to_float(args, obj);
	return ehretval_t::make_float(obj->get_floatval() * operand->get_floatval());
}
EH_METHOD(Float, operator_divide) {
	ASSERT_OBJ_TYPE(float_e, "Float.operator/");
	ehretval_p operand = ehi->to_float(args, obj);
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
