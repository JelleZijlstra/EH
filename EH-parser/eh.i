/*
 * eh.i
 * Jelle Zijlstra, December 2011
 *
 * SWIG interface file for ehi
 */
%{
#include "eh.h"
%}
%module ehphp
void eh_interactive(void);
