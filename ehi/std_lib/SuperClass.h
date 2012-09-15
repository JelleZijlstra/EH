#include "std_lib_includes.h"

// Superclasses (used for inheritance)
class ehsuper_t {
private:
	ehretval_p super_class;
public:
	ehsuper_t(ehretval_p in) : super_class(in) {}

	ehretval_p content() {
		return this->super_class;
	}
};

EH_METHOD(SuperClass, toString);

EXTERN_EHLC(SuperClass)
