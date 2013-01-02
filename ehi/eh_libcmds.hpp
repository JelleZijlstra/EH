/*
 * Header file for EH library commands
 */

#define EH_LIBCMD(name) ehval_p ehlcmd_ ## name(ehval_p obj, ehval_p paras, EHI *ehi)

EH_LIBCMD(quit);
EH_LIBCMD(echo);
EH_LIBCMD(put);
