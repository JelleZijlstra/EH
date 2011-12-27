CCFLAGS = -ggdb -Wall
YACCLEX = \
	yacc -d eh.y; \
	lex eh.l

compiler: eh_compiler.c
	${YACCLEX}
	gcc $(CCFLAGS) lex.yy.c y.tab.c eh_tree.c eh_compiler.c -o ehc

interpreter: eh_interpreter.c
	${YACCLEX}
	gcc $(CCFLAGS) lex.yy.c y.tab.c eh_tree.c eh_libfuncs.c eh_interpreter.c -o ehi
