/*
 * eh_libfuncs.h
 * Jelle Zijlstra, December 2011
 *
 * Function definitions for EH library functions
 */

#define EHLIBFUNC(f) ehretval_p ehlf_ ## f(ehretval_p obj, int nargs, ehretval_p *args, ehcontext_t context, EHI *ehi)

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
EHLIBFUNC(get_type);
EHLIBFUNC(include);
EHLIBFUNC(pow);
EHLIBFUNC(log);
EHLIBFUNC(eval);
EHLIBFUNC(throw);

void printvar_retval(const ehretval_p in);