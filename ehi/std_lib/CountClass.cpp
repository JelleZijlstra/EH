#include "CountClass.h"

EH_INITIALIZER(CountClass) {
	REGISTER_METHOD(CountClass, initialize);
	REGISTER_METHOD(CountClass, docount);
	REGISTER_METHOD(CountClass, setcount);
}

EH_METHOD(CountClass, initialize) {
	return ehretval_t::make_resource(obj->get_full_type(), static_cast<LibraryBaseClass *>(new CountClass()));
}
EH_METHOD(CountClass, docount) {
	ASSERT_TYPE(args, null_e, "CountClass.docount");
	ASSERT_RESOURCE(CountClass, "CountClass.docount");
	return ehretval_t::make_int(++data->count);
}
EH_METHOD(CountClass, setcount) {
	ASSERT_TYPE(args, int_e, "CountClass.setcount");
	ASSERT_RESOURCE(CountClass, "CountClass.setcount");
	data->count = args->get_intval();
	return ehretval_t::make_bool(true);
}
