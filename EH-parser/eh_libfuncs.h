/*
 * eh_libfuncs.h
 * Jelle Zijlstra, December 2011
 *
 * Function definitions for EH library functions
 */
#define EHLIBFUNC(f) void ehlf_ ## f(ehretval_t *paras, ehretval_t *retval, ehcontext_t context)

EHLIBFUNC(getinput);
EHLIBFUNC(printvar);
void printvar_retval(ehretval_t in);