#include "eh.h"

// This line of gibberish serves a purpose.
template<>
garbage_collector<ehretval_t> garbage_collector<ehretval_t>::instance = garbage_collector<ehretval_t>();
