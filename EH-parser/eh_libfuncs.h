/*
 * eh_libfuncs.h
 * Jelle Zijlstra, December 2011
 *
 * Function definitions for EH library functions
 */
#define EHLF_RETFALSE do { \
	*retval = new ehretval_t(false); \
	return; \
} while(0)
#define EHLF_RETTRUE do { \
	*retval = new ehretval_t(true); \
	return; \
} while(0)

#define EHLIBFUNC(f) void ehlf_ ## f(ehretval_t *paras, ehretval_t **retval, ehcontext_t context, EHI *obj)

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
EHLIBFUNC(get_type);
EHLIBFUNC(include);
EHLIBFUNC(pow);
EHLIBFUNC(log);
EHLIBFUNC(eval);

void printvar_retval(const ehretval_t *in);
