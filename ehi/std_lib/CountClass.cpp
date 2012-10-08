#include "CountClass.h"

EH_INITIALIZER(CountClass) {
	REGISTER_METHOD(CountClass, initialize);
	REGISTER_METHOD(CountClass, docount);
	REGISTER_METHOD(CountClass, setcount);
}

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
