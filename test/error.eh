#!/usr/bin/ehi
# Example of an EH file that will throw a syntax error. The error will happen
# because the file lacks a newline after the last statement. I'd like to be
# able to accept such code, but haven't yet been able to coerce yacc to do so.
a = 3
echo a
a = 2