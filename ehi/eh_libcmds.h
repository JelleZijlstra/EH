/*
 * Header file for EH library commands
 */

#define EH_LIBCMD(name) ehretval_p ehlcmd_ ## name(ehretval_p obj, ehretval_p paras, EHI *ehi)

EH_LIBCMD(quit);
EH_LIBCMD(echo);
EH_LIBCMD(put);
