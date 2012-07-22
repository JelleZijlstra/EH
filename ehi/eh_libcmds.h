/*
 * Header file for EH library commands
 */

#define EH_LIBCMD(name) ehretval_p ehlcmd_ ## name(eharray_t *paras)

EH_LIBCMD(quit);
EH_LIBCMD(echo);
EH_LIBCMD(put);
