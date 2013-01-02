#include <stdlib.h>

#include "Random.hpp"

EH_INITIALIZER(Random) {
	REGISTER_METHOD(Random, srand);
	REGISTER_METHOD(Random, rand);
	REGISTER_CONSTANT(Random, max, Integer::make(RAND_MAX));
}

EH_METHOD(Random, srand) {
	args->assert_type<Integer>("Random.srand", ehi);
	srand(args->get<Integer>());
	return nullptr;
}

EH_METHOD(Random, rand) {
	ASSERT_NULL("Random.rand");
	return Integer::make(rand());
}
