CC = gcc
CCFLAGS = -ggdb -Wall
SOURCES = lex.yy.c y.tab.c eh_tree.c eh_error.c
COMPILER = $(SOURCES) eh_compiler.c
COMPILER_O = $(COMPILER:.c=.o)
INTERPRETER = $(SOURCES) eh_libfuncs.c eh_hash.c eh_interpreter.c
EHI = $(INTERPRETER) eh_cli.c
EHI_O = $(EHI:.c=.o)
EHPHP = $(INTERPRETER) eh_php.c eh_wrap.c
EHPHP_O = $(EHPHP:.c=.o)
SYMBOLS = -Wl,-flat_namespace,-U,_zend_register_resource,-U,_zend_rsrc_list_get_rsrc_type,-U,_zend_wrong_param_count,-U,_compiler_globals,-U,_zend_hash_find,-U,_gc_remove_zval_from_buffer,-U,_zend_register_long_constant,-U,_zend_get_constant,-U,__efree,-U,__emalloc,-U,_zend_error,-U,__zend_list_find,-U,_main

eh_wrap.o: eh_wrap.c
	$(CC) `php-config --includes` -fPIC -c $< -o $@

%.o: %.c
	$(CC) -fPIC -c $(CCFLAGS) $< -o $@

compiler: $(COMPILER_O)
	$(CC) $(COMPILER_O) -o ehc

ehi: $(EHI_O)
	$(CC) $(EHI_O) -o ehi

ehphp: $(EHPHP_O)
	$(CC) -dynamiclib $(SYMBOLS) -o ehphp.dylib $(EHPHP_O)
	sudo cp ehphp.dylib /opt/local/lib/php/extensions/no-debug-non-zts-20090626/ehphp.dylib
	sudo cp ehphp.dylib /usr/local/lib/php/extensions/no-debug-non-zts-20100525/ehphp.dylib

y.tab.c y.tab.h: eh.y
	yacc -d eh.y

lex.yy.c: eh.l y.tab.h
	lex eh.l

eh_wrap.c: eh.i
	swig -php eh.i

clean:
	rm -f *.o ehphp.so ehi ehc y.tab.h y.tab.c lex.yy.c y.output eh_wrap.c eh.php
