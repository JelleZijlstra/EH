/*
 * GarbageCollector
 * Provides an interface to the EH garbage collector. This garbage collector
 * currently does not work well, so much will remain uncollected.
 */

#include "GarbageCollector.hpp"

EH_INITIALIZER(GarbageCollector) {
	REGISTER_STATIC_METHOD(GarbageCollector, run);
	REGISTER_STATIC_METHOD(GarbageCollector, stats);
}

/*
 * @description Forces the garbage collector to run immediately.
 * @argument None
 * @returns Null
 */
EH_METHOD(GarbageCollector, run) {
	ehi->get_parent()->gc.do_collect();
	return nullptr;
}

/*
 * @description Prints statistics about the garbage collector, including the
 * number of objects currently allocated in the GC.
 * @argument None
 * @returns Null
 */
EH_METHOD(GarbageCollector, stats) {
	ehi->get_parent()->gc.print_stats();
	return nullptr;
}
