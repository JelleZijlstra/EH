/*
 * eh.h
 * Jelle Zijlstra, December 2011
 *
 * Main header file for the EH scripting language
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "ehi.h"

/*
 * Macros for type checking
 */
#define EH_IS_INT(var) (var.type == int_e)
#define EH_IS_STRING(var) (var.type == string_e)
#define EH_IS_ARRAY(var) (var.type == array_e)
#define EH_IS_BOOL(var) (var.type == bool_e)
#define EH_IS_NULL(var) (var.type == null_e)

/*
 * Top-level functions
 */
void eh_init(void);
void eh_exit(void);
int yylex (void);
void yyerror(char *s);
void *Malloc(size_t size);
void *Calloc(size_t count, size_t size);
void free_node(ehretval_t *in);
ehretval_t *eh_addnode(int operations, int noperations, ...);
ehretval_t eh_execute(ehretval_t *node, ehcontext_t context);
void print_tree(ehretval_t *in, int n);
const char *get_typestring(type_enum type);
void eh_setarg(int argc, char **argv);

// eh_get_x functions
#define GETFUNCPROTO(name, vtype) ehretval_t *eh_get_ ## name(vtype value);
ehretval_t *eh_get_null(void);
GETFUNCPROTO(int, int)
GETFUNCPROTO(string, char *)
GETFUNCPROTO(accessor, accessor_enum)
GETFUNCPROTO(type, type_enum)
GETFUNCPROTO(bool, bool)
GETFUNCPROTO(visibility, visibility_enum)
GETFUNCPROTO(magicvar, magicvar_enum)
GETFUNCPROTO(attribute, attribute_enum)


#include "eh_libfuncs.h"

// indicate that we're returning
extern bool returning;

char *eh_getinput(void);
void eh_interactive(void);
extern bool is_interactive;

