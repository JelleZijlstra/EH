/*
 * eh_libfuncs.h
 * Jelle Zijlstra, December 2011
 *
 * Function definitions for EH library functions
 */
#define EHLF_RETFALSE do { \
	retval->type = bool_e; \
	retval->boolval = false; \
	return; \
} while(0)
#define EHLF_RETTRUE do { \
	retval->type = bool_e; \
	retval->boolval = true; \
	return; \
} while(0)

#define EHLIBFUNC(f) void ehlf_ ## f(ehretval_t *paras, ehretval_t *retval, ehcontext_t context)

EHLIBFUNC(getinput);
EHLIBFUNC(printvar);
EHLIBFUNC(is_null);
EHLIBFUNC(is_string);
EHLIBFUNC(is_int);
EHLIBFUNC(is_bool);
EHLIBFUNC(is_array);
EHLIBFUNC(is_object);
EHLIBFUNC(is_range);
EHLIBFUNC(is_float);
EHLIBFUNC(class_is);
EHLIBFUNC(include);
void printvar_retval(const ehretval_t in);
