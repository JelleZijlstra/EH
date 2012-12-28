#include <stdlib.h>

#include "Random.hpp"

EH_INITIALIZER(Random) {
	REGISTER_METHOD(Random, srand);
	REGISTER_METHOD(Random, rand);
	REGISTER_CONSTANT(Random, max, ehretval_t::make_int(RAND_MAX));
}

EH_METHOD(Random, srand) {
	ASSERT_TYPE(args, int_e, "Random.srand");
	srand(args->get_intval());
	return NULL;
}

EH_METHOD(Random, rand) {
	ASSERT_NULL("Random.rand");
	return ehretval_t::make_int(rand());
}
